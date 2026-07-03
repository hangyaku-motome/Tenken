#pragma once

#include <thread>

#include "HitList.h"
#include "Scanner.h"
#include "types.h"

namespace ScanOp {

void rescanAllHits(const Scanner& ScannerObj, HitList& Hit, std::atomic<float>& progress, TargetTypeT TargetType);

template <typename F> void RunOnScannerThread(std::thread& scannerThread, SessionState& State, F&& task) {
  if (scannerThread.joinable()) scannerThread.join();
  State.IsScanning = true;
  scannerThread = std::thread([&State, task = std::forward<F>(task)]() {
    task();
    State.IsScanning = false;
  });
}

//atp not sure if just giving state as an arg is better or not.
std::vector<HitInfoT> startScan(const Scanner& ScannerObj, const TargetInfoT& TargetInfo, std::atomic<float> & ScanProgress,const std::vector<MapInfoT>& ActiveRegions);
};  // namespace ScanOp
