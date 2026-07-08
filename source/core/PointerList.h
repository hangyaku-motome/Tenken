#pragma once

#include <mutex>

#include "types.h"

class PointerList {
  std::vector<PointerChain> chains_;
  std::mutex mutex_;

public:
  void assignNew(std::vector<PointerChain> new_list);
  const std::vector<PointerChain> get();

  const std::vector<PointerChain>& getRef();
  void lock();
  void unlock();
  bool try_lock();
};
