#pragma once

#include <cstdint>
#include <mutex>
#include <thread>

#include "Scanner.h"
#include "types.h"

class FavouriteList {
  std::vector<FavouriteInfo> favourites_;
  std::mutex mutex_;
  std::thread freeze_thread_;
  std::atomic<bool> freeze_running_;

  void rescanNoLock(const Scanner& scanner_obj, uint64_t index);

public:
  void assignNew(std::vector<FavouriteInfo> new_list);

  void add(const HitInfo& hit, TargetType target_type);
  void remove(uint64_t index);

  void setFreezeDur(uint64_t index, float set_to);
  void setFreezeVal(uint64_t index, const std::vector<uint8_t>& set_to);
  void setFreeze(uint64_t index, bool set_to);
  void setDesc(uint64_t index, std::string set_to);

  void rescan(const Scanner& scanner, uint64_t index);
  void rescanAll(const Scanner& scanner);

  void write(const Scanner& scanner, uint64_t index, const std::vector<uint8_t>& value);

  std::vector<FavouriteInfo> getAll();

  void startFreezeThread(const Scanner& scanner);

  void endFreezeThread();

  void reset();

  const std::vector<FavouriteInfo>& getRef();
  void lock();
  void unlock();
  bool try_lock();
};
