#include "PointerList.h"

#include <algorithm>

#include "Log.h"
#include "types.h"
#include "version.h"

void PointerList::openFile(const std::filesystem::path& path) {
  if (stream_.is_open()) stream_.close();

  stream_.open(path, std::ifstream::in | std::ifstream::binary);

  uint64_t magic_bytes = 0;
  uint8_t file_version = 0;
  uint8_t entry_size = 0;
  uint64_t entry_start_point = 0;
  int8_t save_index = -1;

  stream_.read(reinterpret_cast<char*>(&magic_bytes), sizeof(magic_bytes));
  if (magic_bytes != Pointer::magic_bytes) {
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
    Log::error("Entry size and pointer chain do not line up....Something's fishy here!");
    status_ = -1;
    return;
  };

  stream_.read(reinterpret_cast<char*>(&entry_start_point), sizeof(entry_start_point));
  if (entry_start_point == 0) {
    Log::error("Header size is 0. That should be impossible.");
    status_ = -1;
    return;
  }

  stream_.read(reinterpret_cast<char*>(&save_index), sizeof(save_index));

  if (save_index < 0) {
    Log::error("Save index couldn't be read...Uhm...Well, this one doesn't actually matter that much so I'll let it "
               "slide. I'll just set it to 0.");
    save_index_ = 0;
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

std::vector<Pointer::Chain> PointerList::getFromRaw(uint64_t start_index, uint64_t read_count) {
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
  status_ = 0;
  stream_.close();
};
