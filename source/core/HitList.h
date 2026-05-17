#pragma once

#include "Scanner.h"
#include "types.h"
#include <cstdint>
#include <mutex>
#include <vector>

class HitList {

  std::vector<HitInfoT> hits_;
  std::mutex mutex_;

public:
  void assignNew(const std::vector<HitInfoT> &NewHits);

  void rescan(const Scanner &ScannerObj, uint64_t index,
              TargetTypeT TargetType);

  void write(const Scanner &ScannerObj, uint64_t index,
             const std::vector<uint8_t> &value);

  void filter(RelativeStatus KeepType);
  void filter(const std::vector<uint8_t> &KeepValue);

  uint64_t count();

  const std::vector<HitInfoT> &getAll() const { return hits_; }
  const HitInfoT &getIndex(uint64_t index) const { return hits_[index]; }

  void reset() { hits_.clear(); }
};
