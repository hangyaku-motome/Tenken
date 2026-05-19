#pragma once

#include "Scanner.h"
#include "types.h"
#include <cstdint>
#include <mutex>
#include <thread>

class FavouriteList {

  std::vector<FavouriteInfoT> favourites_;
  std::mutex mutex_;
  std::thread freezeThread;
  std::atomic<bool> freezeRunning;

  void rescanNoLock(const Scanner &ScannerObj, uint64_t index, TargetTypeT TargetType);

public:
  void assignNew(const std::vector<FavouriteInfoT> &NewList);

  void add(const HitInfoT &hit, TargetTypeT TargetType);
  void remove(uint64_t index);

  void setFreezeDur(uint64_t index, float setTo);
  void setFreezeVal(uint64_t index, const std::vector<uint8_t> &setTo);
  void setFreeze(uint64_t index, bool setTo);
  void setDesc(uint64_t index, std::string setTo);

  void rescan(const Scanner &ScannerObj, uint64_t index, TargetTypeT TargetType);
  void rescanAll(const Scanner &ScannerObj, TargetTypeT TargetType);

  void write(const Scanner &ScannerObj, uint64_t index, const std::vector<uint8_t> &value);

  const std::vector<FavouriteInfoT> &get() { return favourites_; }

  void startFreezeThread(const Scanner &ScannerObj);

  void endFreezeThread() {
    freezeRunning = false;
    if (freezeThread.joinable())
      freezeThread.join();
  }

  void reset() {
    favourites_.clear();
    endFreezeThread();
  }

  // add freeze thread and funcs.
};