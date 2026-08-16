#pragma once

#include <thread>

#include "HitList.h"
#include "Scanner.h"
#include "types.h"

namespace ScanOp {

void rescanAllHits(const Scanner& scanner, HitList& hit, std::atomic<float>& progress, TargetType target_type);

template <typename F> void runOnScannerThread(std::thread& scanner_thread, State& state, ScanType scan_type, F&& task) {
  if (scanner_thread.joinable()) scanner_thread.join();
  state.scan_type = scan_type;
  scanner_thread = std::thread([&state, task = std::forward<F>(task)]() {
    task();
    state.scan_type = ScanType::Nothing;
  });
}

// atp not sure if just giving state as an arg is better or not.
std::vector<HitInfo> startScan(const Scanner& scanner_obj,
                               const TargetInfo& target_info,
                               std::atomic<float>& scanner_progress,
                               const std::vector<MapInfo>& active_regions);
};  // namespace ScanOp
