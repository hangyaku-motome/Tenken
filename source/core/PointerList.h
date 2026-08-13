#pragma once

#include <filesystem>
#include <fstream>

#include "types.h"

class PointerList {
  std::ifstream stream_;

  uint64_t entry_start_point_;

  // 0 when closed, 1 when open and valid, -1 when open but error.
  int32_t status_ = 0;

public:
  void openFile(const std::filesystem::path& path);

  void close();

  int32_t getStatus() { return status_; };

  // set when a file is closed and another one is opened right after. couldn't figure out another way to express this.
  // After being acknowledged should be set to false.
  bool just_opened_ = false;

  std::vector<std::string> data_module_names_;
  uint64_t total_chains_;

  std::vector<Pointer::PrettyChain> getFrom(uint64_t start_index, uint64_t read_count);
  std::vector<Pointer::Chain> getFromRaw(uint64_t start_index, uint64_t read_count);
};
