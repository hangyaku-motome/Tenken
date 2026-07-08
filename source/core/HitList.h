#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

#include "Scanner.h"
#include "types.h"

class HitList {
  std::vector<HitInfo> cached_hits_;  // assigning hits_ to this in filter
  std::vector<HitInfo> hits_;
  std::mutex mutex_;

public:
  void assignNew(std::vector<HitInfo> new_hits);

  void rescan(const Scanner& scanner, uint64_t index, TargetType target_type);

  void write(const Scanner& scanner_obj, uint64_t index, const std::vector<uint8_t>& value);

  void filter(RelativeStatus keep_type);
  void filter(const std::vector<uint8_t>& keep_type);

  uint64_t count();

  const std::vector<HitInfo> getAll();

  const HitInfo getIndex(uint64_t index);

  void reset();

  void restore_old_hits();

  const std::vector<HitInfo>& getRef();
  void lock();
  void unlock();
  bool try_lock();
};
