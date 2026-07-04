#pragma once

#include <thread>

#include "HitList.h"
#include "Scanner.h"
#include "types.h"

namespace ScanOp {

void rescanAllHits(const Scanner& scanner, HitList& hit, std::atomic<float>& progress, TargetType target_type);

template <typename F> void runOnScannerThread(std::thread& scanner_thread, SessionState& state, F&& task) {
  if (scanner_thread.joinable()) scanner_thread.join();
  state.is_scanning = true;
  scanner_thread = std::thread([&state, task = std::forward<F>(task)]() {
    task();
    state.is_scanning = false;
  });
}

//atp not sure if just giving state as an arg is better or not.
std::vector<HitInfoT> startScan(const Scanner& scanner_obj, const TargetInfoT& target_info, std::atomic<float> & scanner_progress,const std::vector<MapInfoT>& active_regions);
};  // namespace ScanOp
