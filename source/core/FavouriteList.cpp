#include "FavouriteList.h"
#include "Scanner.h"
#include "types.h"
#include "utils.h"

void FavouriteList::assignNew(const std::vector<FavouriteInfoT> &NewList) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_ = NewList;
}

void FavouriteList::add(const HitInfoT &hit, TargetTypeT TargetType) {
  std::scoped_lock<std::mutex> lock(mutex_);
  FavouriteInfoT PushFavourite;
  favourites_.push_back({.location = hit.location,
                         .value = hit.value,
                         .previous_value = hit.previous_value,
                         .desc = "",
                         .bytes_around = hit.bytes_around,
                         .type = TargetType,
                         .frozen_value = hit.value});
}

void FavouriteList::remove(uint64_t index) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_.erase(favourites_.begin() + static_cast<int64_t>(index));
}

void FavouriteList::setFreeze(uint64_t index, bool setTo) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_[index].frozen = setTo;
}

void FavouriteList::setDesc(uint64_t index, std::string setTo) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_[index].desc = setTo;
}

void FavouriteList::rescanNoLock(const Scanner &ScannerObj, uint64_t index,
                                 TargetTypeT TargetType) {
  favourites_[index].previous_value = favourites_[index].value;

  favourites_[index].bytes_around.resize(BYTES_BEFORE + BYTES_AFTER +
                                         favourites_[index].value.size());

  favourites_[index].bytes_around =
      ScannerObj.readAdr(favourites_[index].location - BYTES_BEFORE,
                         favourites_[index].bytes_around.size());

  if (favourites_[index].bytes_around.size() !=
      BYTES_BEFORE + BYTES_AFTER + favourites_[index].value.size()) {
    favourites_[index].bytes_around.clear();
    favourites_[index].value = ScannerObj.readAdr(
        favourites_[index].location, favourites_[index].value.size());
    if (favourites_[index].value.empty()) {
      favourites_.erase(favourites_.begin() + index);
      return;
    }
  } else {
    favourites_[index].value.assign(
        favourites_[index].bytes_around.begin() + BYTES_BEFORE,
        favourites_[index].bytes_around.begin() + BYTES_BEFORE +
            static_cast<int64_t>(favourites_[index].value.size()));
  }
  if (!favourites_[index].previous_value.empty())
    favourites_[index].status = tagChange(
        reinterpret_cast<const char *>(favourites_[index].value.data()),
        reinterpret_cast<const char *>(
            favourites_[index].previous_value.data()),
        TargetType, favourites_[index].value.size());
}

void FavouriteList::rescan(const Scanner &ScannerObj, uint64_t index,
                           TargetTypeT TargetType) {
  std::scoped_lock<std::mutex> lock(mutex_);
  rescanNoLock(ScannerObj, index, TargetType);
}

void FavouriteList::rescanAll(const Scanner &ScannerObj,
                              TargetTypeT TargetType) {
  std::scoped_lock<std::mutex> lock(mutex_);

  for (uint64_t i = 0; i < favourites_.size(); ++i)
    rescanNoLock(ScannerObj, i, TargetType);
}

void FavouriteList::write(const Scanner &ScannerObj, uint64_t index,
                          const std::vector<uint8_t> &value) {
  {
    std::scoped_lock<std::mutex> lock(mutex_);
    ScannerObj.writeAdr(favourites_[index].location, value);
  }
  setFreezeVal(index, value);
}

void FavouriteList::setFreezeVal(uint64_t index,
                                 const std::vector<uint8_t> &setTo) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_[index].frozen_value = setTo;
}

void FavouriteList::startFreezeThread(const Scanner &ScannerObj) {
  if (freezeThread.joinable())
    freezeThread.join();
  freezeRunning = true;
  freezeThread = std::thread([&]() {
    while (freezeRunning) {
      {
        std::scoped_lock<std::mutex> lock(mutex_);
        for (const auto &favourite : favourites_)
          if (favourite.frozen)
            ScannerObj.writeAdr(favourite.location, favourite.frozen_value);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
  });
}