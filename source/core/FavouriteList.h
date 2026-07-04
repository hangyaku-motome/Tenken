#pragma once

#include <cstdint>
#include <mutex>
#include <thread>

#include "Scanner.h"
#include "types.h"

class FavouriteList {
  std::vector<FavouriteInfoT> favourites_;
  std::mutex mutex_;
  std::thread freeze_thread_;
  std::atomic<bool> freeze_running_;

  void rescanNoLock(const Scanner& scanner_obj, uint64_t index, TargetType target_type);

public:
  void assignNew(const std::vector<FavouriteInfoT>& new_list
                 );

  void add(const HitInfoT& hit, TargetType target_type);
  void remove(uint64_t index);

  void setFreezeDur(uint64_t index, float set_to);
  void setFreezeVal(uint64_t index, const std::vector<uint8_t>& set_to);
  void setFreeze(uint64_t index, bool set_to);
  void setDesc(uint64_t index, std::string set_to);

  void rescan(const Scanner& scanner, uint64_t index, TargetType target_type);
  void rescanAll(const Scanner& scanner, TargetType target_type);

  void write(const Scanner& scanner, uint64_t index, const std::vector<uint8_t>& value);

  const std::vector<FavouriteInfoT>& get() { return favourites_; }

  void startFreezeThread(const Scanner& scanner);

  void endFreezeThread() {
    freeze_running_ = false;
    if (freeze_thread_.joinable()) freeze_thread_.join();
  }

  void reset() {
    favourites_.clear();
    endFreezeThread();
  }

  // add freeze thread and funcs.
};
