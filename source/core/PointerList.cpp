#include "PointerList.h"

#include <algorithm>

#include "Log.h"
#include "types.h"
#include "version.h"

// TODO:we couldd also just check is_file_open instead of returning a bool but..will look into this later
bool PointerList::openFile(const std::filesystem::path& path) {
  if (stream_.is_open()) stream_.close();

  stream_.open(path, std::ifstream::in | std::ifstream::binary);

  uint64_t magic_bytes = 0;
  uint8_t file_version = 0;
  uint8_t entry_size = 0;
  uint64_t entry_start_point = 0;

  stream_.read(reinterpret_cast<char*>(&magic_bytes), sizeof(magic_bytes));
  if (magic_bytes != Pointer::magic_bytes) {
    Log::error("This is not the right file type. \".tptr\" files onlyyy...");
    return false;
  };

  stream_.read(reinterpret_cast<char*>(&file_version), sizeof(file_version));
  if (file_version != PointerResultVersion) {
    Log::error("This is from another version. Sorry, can't parse.");
    return false;
  };

  stream_.read(reinterpret_cast<char*>(&entry_size), sizeof(entry_size));
  if (entry_size != sizeof(Pointer::Chain)) {
    Log::error("Entry size and pointer chain do not line up. This is most definitely a bug. OR, you got this result "
               "on a different platform/system and now tried to use it in another one.");
    return false;
  };

  stream_.read(reinterpret_cast<char*>(&entry_start_point), sizeof(entry_start_point));
  if (entry_start_point == 0) {
    Log::error("Header size is 0. How...Well this means there were no pointers in the file or there's a bug.");
    return false;
  }

  // after this should be headers.

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
    Log::error("No module names found in file...This is most probably a bug.");  // NOTE: what do when no pointer?
    return false;
  }

  stream_.seekg(0, std::ios::end);
  uint64_t file_size = static_cast<uint64_t>(stream_.tellg());
  total_chains_ = (file_size - entry_start_point) / entry_size;

  entry_start_point_ = entry_start_point;

  if (!stream_) {
    Log::error("Reading file failed.");
    return false;
  }

  stream_.clear();
  return true;
}

std::vector<Pointer::PrettyChain> PointerList::getFrom(uint64_t start_index, uint64_t read_count) {
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
  if (!stream_.is_open()) return {};
  std::vector<Pointer::Chain> chains(read_count);
  stream_.seekg(entry_start_point_ + static_cast<std::streamoff>(start_index) * sizeof(Pointer::Chain));
  stream_.read(reinterpret_cast<char*>(chains.data()), chains.size() * sizeof(Pointer::Chain));
  if (!stream_) {
    Log::error("couldn't get from file...");
    // maybee clear stream_ or not.
    return {};
  }

  // chain.valid_offset being 0 would be a problem with the file but things can get wonky if it ever ends up here
  for (auto& chain : chains) std::reverse(chain.offsets.begin(), chain.offsets.begin() + chain.valid_offsets);

  return chains;
}
