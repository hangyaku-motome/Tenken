#pragma once

// TODO: :if total chains is less than x, we can just get all of it. (we can have hundreds of thousands of hits. We
// should be willing to hold a similar amount of chains too.)

// Why did I have a lobotomy and forget my naming scheme ?
#include <filesystem>
#include <fstream>

#include "types.h"

class PointerList {
  std::ifstream stream_;

  uint64_t entry_start_point_;

public:
  bool openFile(const std::filesystem::path& path);
  std::vector<std::string> data_module_names_;
  bool is_file_open_ = false;

  uint64_t total_chains_;
  std::vector<Pointer::PrettyChain> getFrom(uint64_t start_index, uint64_t read_count);

  std::vector<Pointer::Chain> getFromRaw(uint64_t start_index, uint64_t read_count);

  // another one to getRawFrom
};
