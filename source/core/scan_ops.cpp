#include "scan_ops.h"
#include "HitList.h"
#include "Scanner.h"
#include "types.h"

void ScanOp::rescanAllHits(const Scanner &ScannerObj, HitList &Hit,
                           std::atomic<float> &progress,
                           const TargetTypeT TargetType) {
  auto hit_count = Hit.count();

  for (uint64_t i = 0; i < hit_count; ++i) {
    Hit.rescan(ScannerObj, i, TargetType);
    hit_count = Hit.count();
    progress = static_cast<float>(i) / hit_count;
  }
}