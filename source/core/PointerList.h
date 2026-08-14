#pragma once

#include <filesystem>
#include <fstream>

#include "types.h"

class PointerList {
  std::ifstream stream_;

  uint64_t entry_start_point_;

  // 0 when closed, 1 when open and valid, -1 when open but error.
  int8_t status_ = 0;
  uint8_t filter_index_ = 0;  // which iteration? first scan, 1st filter, 2nd filter, 3rd filter..........
  uint8_t depth_ = 0;
  TargetType target_type_ = TargetType::invalid;

public:
  void openFile(const std::filesystem::path& path);

  void close();

  int32_t getStatus() { return status_; };

  uint8_t getSaveIndex() { return filter_index_; };

  uint8_t getDepth() { return depth_; };

  TargetType getTargetType() { return target_type_; }

  // set when a file is closed and another one is opened right after. couldn't figure out another way to express this.
  // After being acknowledged should be set to false.
  bool just_opened_ = false;

  std::vector<std::string> data_module_names_;
  uint64_t total_chains_;

  std::vector<Pointer::PrettyChain> getFrom(uint64_t start_index, uint64_t read_count);
  Pointer::PrettyChain get(uint64_t index);
  Pointer::Chain getRaw(uint64_t index);
  std::vector<Pointer::Chain> getFromRaw(uint64_t start_index, uint64_t read_count);
};
