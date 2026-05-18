#pragma once

#include "HitList.h"
#include "Scanner.h"
#include "types.h"
#include <thread>

namespace ScanOp {

void rescanAllHits(const Scanner &ScannerObj, HitList &Hit,
                   std::atomic<float> &progress, TargetTypeT TargetType);

template <typename F>
void RunOnScannerThread(std::thread &scannerThread, SessionState &State,
                        F &&task) {
  if (scannerThread.joinable())
    scannerThread.join();
  State.IsScanning = true;
  scannerThread = std::thread([&State, task = std::forward<F>(task)]() {
    task();
    State.IsScanning = false;
  });
}

std::vector<HitInfoT> startScan(const Scanner &ScannerObj, SessionState &State);
}; // namespace ScanOp