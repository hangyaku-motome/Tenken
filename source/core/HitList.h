#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

#include "Scanner.h"
#include "types.h"

class HitList {
  std::vector<HitInfoT> cached_hits_;  // assigning hits_ to this in filter
  std::vector<HitInfoT> hits_;
  std::mutex mutex_;

public:
  void assignNew(const std::vector<HitInfoT>& new_hits);

  void rescan(const Scanner& scanner, uint64_t index, TargetType target_type);

  void write(const Scanner& scanner_obj, uint64_t index, const std::vector<uint8_t>& value);

  void filter(RelativeStatus keep_type);
  void filter(const std::vector<uint8_t>& keep_type);

  uint64_t count();

  const std::vector<HitInfoT>& getAll() const { return hits_; }

  const HitInfoT& getIndex(uint64_t index) const { return hits_[index]; }

  void reset() { hits_.clear(); }

  void restore_old_hits() { hits_ = cached_hits_; }
};
