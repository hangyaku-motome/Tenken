#pragma once

#include <filesystem>
#include <fstream>

#include "types.h"

class PointerList {
  std::ifstream stream_;

  uint64_t entry_start_point_;

public:
  bool openFile(const std::filesystem::path& path);

  bool isOpen() { return stream_.is_open(); };

  bool failed() { return static_cast<bool>(!stream_); };

  void close() {
    stream_.clear();
    stream_.close();
  };

  std::vector<std::string> data_module_names_;
  uint64_t total_chains_;

  std::vector<Pointer::PrettyChain> getFrom(uint64_t start_index, uint64_t read_count);
  std::vector<Pointer::Chain> getFromRaw(uint64_t start_index, uint64_t read_count);
};
