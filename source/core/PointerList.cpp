#include "PointerList.h"

#include <fcntl.h>

#include <algorithm>
#include <mutex>

#include "Log.h"
#include "types.h"
#include "version.h"

void PointerList::openFile(const std::filesystem::path& path) {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  if (stream_.is_open()) stream_.close();

  stream_.open(path, std::ifstream::in | std::ifstream::binary);

  uint64_t magic_bytes = 0;
  uint64_t entry_start_point = 0;
  uint8_t file_version = 0;
  uint8_t entry_size = 0;
  int8_t filter_index = -1;
  uint8_t depth = 0;
  TargetType target_type = TargetType::invalid;
  uint8_t target_size = 0;

  stream_.read(reinterpret_cast<char*>(&magic_bytes), sizeof(magic_bytes));
  if (magic_bytes != Pointer::MagicBytes) {
    Log::error("This is not the right file type. \".tptr\" files onlyyy...");
    status_ = -1;
    return;
  };

  stream_.read(reinterpret_cast<char*>(&file_version), sizeof(file_version));
  if (file_version != PointerResultVersion) {
    Log::error("This is from another version. Sorry, can't parse.");
    status_ = -1;
    return;
  };

  stream_.read(reinterpret_cast<char*>(&entry_size), sizeof(entry_size));
  if (entry_size != sizeof(Pointer::Chain)) {
    Log::error("Entry size and pointer chain do not line up....Something's fishy here! Can't parse it.");
    status_ = -1;
    return;
  };

  stream_.read(reinterpret_cast<char*>(&depth), sizeof(depth));
  if (depth > Pointer::MaxDepth || depth == 0) {
    Log::error(
        "Depth is invalid...Not the end of the world. I'm setting it to max depth so your table will look ugly!");
    depth_ = Pointer::MaxDepth;
  }
  depth_ = depth;

  stream_.read(reinterpret_cast<char*>(&filter_index), sizeof(filter_index));
  if (filter_index == -1) {
    Log::error("Save index couldn't be read...Uhm...Well, this one doesn't actually matter that much so I'll let it "
               "slide. I'll just set it to 0.");
    filter_index_ = 0;
  }
  filter_index_ = filter_index;

  stream_.read(reinterpret_cast<char*>(&target_type), sizeof(target_type));
  if (target_type == TargetType::invalid) {
    Log::error(
        "Target type couldn't be read...This is not pointer logic breaking, however you won't be able to edit it's "
        "value or...really even view it when you add it to favourites. For now, this translates to \"I can't parse!\"");
    status_ = -1;
    return;
  }
  target_type_ = target_type;

  stream_.read(reinterpret_cast<char*>(&target_size), sizeof(target_size));
  if (target_size == 0) {
    Log::error("Target size is invalid...Won't parsee");
    status_ = -1;
    return;
  }
  target_size_ = target_size;

  stream_.read(reinterpret_cast<char*>(&entry_start_point), sizeof(entry_start_point));
  if (entry_start_point == 0) {
    Log::error("Entry start point being 0 should not be possible..Can't parse.");
    status_ = -1;
    return;
  }

  // after this should be module headers.

  uint64_t table_size = entry_start_point - static_cast<uint64_t>(stream_.tellg());

  std::vector<uint8_t> table_data(table_size);
  stream_.read(reinterpret_cast<char*>(table_data.data()), table_size);

  for (uint64_t i = 0; i < table_size;) {
    uint8_t len;
    memcpy(&len, table_data.data() + i, sizeof(len));

    std::string name(table_data.begin() + i + sizeof(len), table_data.begin() + i + sizeof(len) + len);

    data_module_names_.push_back(name);

    i += sizeof(len) + name.length();
  }

  if (data_module_names_.empty()) {
    status_ = -1;
    Log::error("No module names found in file...");
    return;
  }

  stream_.seekg(0, std::ios::end);
  uint64_t file_size = static_cast<uint64_t>(stream_.tellg());
  total_chains_ = (file_size - entry_start_point) / entry_size;

  entry_start_point_ = entry_start_point;

  if (!stream_) {
    status_ = -1;
    Log::error("Reading file failed.");
    return;
  }

  status_ = 1;
  just_opened_ = true;
  return;
}

std::vector<Pointer::PrettyChain> PointerList::getFrom(uint64_t start_index, uint64_t read_count) {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  if (status_ != 1) return {};  // redundant since getFromRaw calls it but let's be explicit.
  auto chains_raw = getFromRaw(start_index, read_count);
  if (chains_raw.empty()) return {};

  std::vector<Pointer::PrettyChain> chains;
  chains.reserve(chains_raw.size());
  for (const auto& chain_raw : chains_raw) {
    std::vector<int64_t> vec(chain_raw.offsets.begin(), chain_raw.offsets.begin() + chain_raw.valid_offsets);
    chains.push_back({.offsets = vec,
                      .module_name = data_module_names_[chain_raw.module_id],
                      .offset_in_module = chain_raw.offset_in_module});
  }

  return chains;
}

Pointer::PrettyChain PointerList::getFrom(uint64_t index) {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  if (status_ != 1) return {};
  auto chains = getFromRaw(index, 1);

  if (chains.empty()) return {};

  std::vector<int64_t> vec(chains[0].offsets.begin(), chains[0].offsets.begin() + chains[0].valid_offsets);
  return {.offsets = vec,
          .module_name = data_module_names_[chains[0].module_id],
          .offset_in_module = chains[0].offset_in_module};
}

Pointer::Chain PointerList::getFromRaw(uint64_t index) {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  if (status_ != 1) return {};
  auto chains = getFromRaw(index, 1);

  if (chains.empty()) return {};

  return chains.front();
}

std::vector<Pointer::Chain> PointerList::getFromRaw(uint64_t start_index, uint64_t read_count) {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  if (status_ != 1) return {};
  std::vector<Pointer::Chain> chains(read_count);
  stream_.seekg(entry_start_point_ + static_cast<std::streamoff>(start_index) * sizeof(Pointer::Chain));
  stream_.read(reinterpret_cast<char*>(chains.data()), chains.size() * sizeof(Pointer::Chain));
  if (!stream_) {
    Log::error("couldn't get from file...");
    status_ = -1;
    return {};
  }

  for (auto& chain : chains)
    if (chain.valid_offsets > 0) std::reverse(chain.offsets.begin(), chain.offsets.begin() + chain.valid_offsets);

  return chains;
}

void PointerList::close() {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  status_ = 0;
  stream_.close();
};

int32_t PointerList::getStatus() {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  return status_;
};

uint8_t PointerList::getSaveIndex() {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  return filter_index_;
};

uint8_t PointerList::getDepth() {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  return depth_;
};

TargetType PointerList::getTargetType() {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  return target_type_;
}

uint8_t PointerList::getTargetSize() {
  std::scoped_lock<std::recursive_mutex> lock(mutex_);
  return target_size_;
}
