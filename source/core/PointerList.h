#pragma once

// TODO: :if total chains is less than x, we can just get all of it. (we can have hundreds of thousands of hits. We
// should be filling to hold a similar amount of chains too.)
#include <filesystem>
#include <fstream>

#include "types.h"

class PointerList {
  std::ifstream stream_;

  std::vector<std::string> data_module_names_;
  uint64_t entry_start_point_;

public:
  bool open_file(const std::filesystem::path& path);
  bool is_file_open_ = false;

  bool compare_to_file(const std::filesystem::path& path);

  uint64_t total_chains_;
  std::vector<Pointer::PrettyChain> get_from(uint64_t start_index, uint64_t read_count);
};
