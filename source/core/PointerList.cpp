#include "PointerList.h"

#include <mutex>

const std::vector<PointerChain> PointerList::get() {
  std::scoped_lock<std::mutex> lock(mutex_);
  return chains_;
}

void PointerList::assignNew(std::vector<PointerChain> new_list) {
  std::scoped_lock<std::mutex> lock(mutex_);
  chains_ = std::move(new_list);
}

bool PointerList::try_lock() { return mutex_.try_lock(); }

void PointerList::lock() { mutex_.lock(); }

void PointerList::unlock() { mutex_.unlock(); }

// don't forget to call lock before and unlock after...
const std::vector<PointerChain>& PointerList::getRef() { return chains_; }
