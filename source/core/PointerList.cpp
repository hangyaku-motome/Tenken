#include "PointerList.h"

#include <iostream>

#include "LogW.h"
#include "types.h"

/// PLEASEEE WORK ALREADYYYYYYYYYYYYYYYYYYYYYY

bool PointerList::open_file(const std::filesystem::path& path) {
  printf("trying to open file. good luck me, hope I don't mysteriously sigfault or something!\n");
  if (stream_.is_open()) stream_.close();

  stream_.open(path, std::ifstream::in | std::ifstream::binary);
  stream_.exceptions(std::ios::failbit | std::ios::badbit);

  try {
    uint64_t magic_bytes = 0;
    uint8_t file_version = 0;
    uint8_t entry_size = 0;
    uint64_t entry_start_point = 0;

    stream_.read(reinterpret_cast<char*>(&magic_bytes), sizeof(magic_bytes));
    if (magic_bytes != Pointer::magic_bytes) {
      Log::error("This is not the right file type.");
      is_file_open_ = false;
      return false;
    };

    stream_.read(reinterpret_cast<char*>(&file_version), sizeof(file_version));
    if (file_version != Pointer::file_version) {
      Log::error("This is from another version. Sorry, can't parse.");
      is_file_open_ = false;
      return false;
    };

    stream_.read(reinterpret_cast<char*>(&entry_size), sizeof(entry_size));
    if (entry_size != sizeof(Pointer::Chain)) {
      Log::error("Entry size and pointer chain do not line up. This is most definitely a bug.");
      is_file_open_ = false;
      return false;
    };

    stream_.read(reinterpret_cast<char*>(&entry_start_point), sizeof(entry_start_point));
    if (entry_start_point == 0) {
      Log::error("Header size is 0. How...Well this means there were no pointers in the file or there's a bug.");
      is_file_open_ = false;
      return false;
    }

    // after this should be headers.

    uint64_t table_size = entry_start_point - static_cast<uint64_t>(stream_.tellg());

    std::cout << table_size << "\n";

    std::vector<uint8_t> table_data(table_size);
    stream_.read(reinterpret_cast<char*>(table_data.data()), table_size);

    for (const auto& byte : table_data) std::cout << " " << byte << " ";
    std::cout << "\n\n";

    for (uint64_t i = 0; i < table_size;) {
      uint8_t len;
      memcpy(&len, table_data.data() + i, sizeof(len));
      std::cout << "len: " << len << "\n";

      std::string name(table_data.begin() + i + sizeof(len), table_data.begin() + i + sizeof(len) + len);
      std::cout << "string in question: " << name << "\n";

      data_module_names_.push_back(name);

      i += sizeof(len) + name.length();
    }

    if (data_module_names_.empty()) {
      Log::error("No module names found in file...This is most probably a bug.");
      is_file_open_ = false;
      return false;
    }

    stream_.seekg(0, std::ios::end);
    uint64_t file_size = static_cast<uint64_t>(stream_.tellg());
    total_chains_ = (file_size - entry_start_point) / entry_size;

    entry_start_point_ = entry_start_point;

    std::cout << "WHAT I PARSED INFO. " << "MAGIC BYTES: " << std::hex << magic_bytes << "\n"
              << "FILE VERSION :" << std::dec << file_version << "\n"
              << "ENTRY SIZE: " << entry_size << "\n"
              << "ENTRY START POINT: " << std::hex << entry_start_point << "\n"
              << "TABLE SIZE: " << std::dec << table_size << "\n"
              << "DATA MODULE NAMES:" << data_module_names_[0] << "\n\n";
  } catch (...) {
    Log::error("Reading file failed.");
    is_file_open_ = false;
    return false;
  }

  is_file_open_ = true;
  return true;
}

std::vector<Pointer::PrettyChain> PointerList::get_from(uint64_t start_index, uint64_t read_count) {
  printf("trying to get from\n");
  std::vector<Pointer::Chain> chains_buf(read_count);
  stream_.seekg(entry_start_point_ + static_cast<std::streamoff>(start_index) * sizeof(Pointer::Chain));
  stream_.read(reinterpret_cast<char*>(chains_buf.data()), chains_buf.size() * sizeof(Pointer::Chain));

  std::cout << "\n\nso this is what I read from stream in get_from into chains_buf:\n\n";
  for (const auto& chain : chains_buf) {
    std::cout << "id " << chain.module_id << "\n"
              << "offset " << chain.offset_in_module << "\n"
              << "valid offsets" << static_cast<int32_t>(chain.valid_offsets) << "\n";
    for (int i = 0; i < chain.valid_offsets; ++i) std::cout << "offset: " << chain.offsets[i] << "\n";
    std::cout << "\n";
  }

  std::cout << "\n\n\n";

  std::vector<Pointer::PrettyChain> chains(read_count);
  for (const auto& chain_buf : chains_buf) {
    std::vector<int64_t> vec(chain_buf.offsets.begin(), chain_buf.offsets.begin() + chain_buf.valid_offsets);
    chains.push_back({.offsets = vec,
                      .module_name = data_module_names_[chain_buf.module_id],
                      .offset_in_module = chain_buf.offset_in_module});
  }

  return chains;
}
