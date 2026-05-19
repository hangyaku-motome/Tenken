#include "HitList.h"
#include "LogW.h"
#include "types.h"
#include "utils.h"
#include <algorithm>

void HitList::assignNew(const std::vector<HitInfoT> &NewHits) {
  std::scoped_lock<std::mutex> lock(mutex_);
  hits_ = NewHits;
}

void HitList::rescan(const Scanner &ScannerObj, uint64_t index, const TargetTypeT TargetType) {
  std::scoped_lock<std::mutex> lock(mutex_);
  hits_[index].previous_value = hits_[index].value;

  hits_[index].bytes_around.resize(BYTES_BEFORE + BYTES_AFTER + hits_[index].value.size());

  hits_[index].bytes_around =
      ScannerObj.readAdr(hits_[index].location - BYTES_BEFORE, hits_[index].bytes_around.size());

  if (hits_[index].bytes_around.size() != BYTES_BEFORE + BYTES_AFTER + hits_[index].value.size()) {
    hits_[index].bytes_around.clear();
    hits_[index].value = ScannerObj.readAdr(hits_[index].location, hits_[index].value.size());
    if (hits_[index].value.empty()) {
      hits_.erase(hits_.begin() + static_cast<int64_t>(index));
      return;
    }
  } else {
    hits_[index].value.assign(hits_[index].bytes_around.begin() + BYTES_BEFORE,
                              hits_[index].bytes_around.begin() + BYTES_BEFORE +
                                  static_cast<int64_t>(hits_[index].value.size()));
  }
  if (!hits_[index].previous_value.empty())
    dispatchType(TargetType, [&]<typename T> {
      hits_[index].status =
          tagChange(datatoType<T>(hits_[index].value), datatoType<T>(hits_[index].previous_value));
    });
}

void HitList::write(const Scanner &ScannerObj, uint64_t index, const std::vector<uint8_t> &value) {
  std::scoped_lock<std::mutex> lock(mutex_);
  ScannerObj.writeAdr(hits_[index].location, value);
}

void HitList::filter(RelativeStatus KeepType) {
  std::scoped_lock<std::mutex> lock(mutex_);
  RelativeStatus KeepType2 = KeepType;
  RelativeStatus KeepType3 = KeepType;
  if (KeepType == RelativeStatus::CHANGED) {
    KeepType2 = RelativeStatus::INCREASED;
    KeepType3 = RelativeStatus::DECREASED;
  }

  cachedhits_ = hits_;

  uint64_t init_amount = hits_.size();

  hits_.erase(std::remove_if(hits_.begin(), hits_.end(),
                             [KeepType, KeepType2, KeepType3](const HitInfoT &hit) {
                               return hit.status != KeepType && hit.status != KeepType2 &&
                                      hit.status != KeepType3;
                             }),
              hits_.end());
  Log::Info(std::to_string(hits_.size()) + " Hits left. (" +
            std::to_string(init_amount - hits_.size()) + " filtered.)");
}

void HitList::filter(const std::vector<uint8_t> &KeepValue) {
  std::scoped_lock<std::mutex> lock(mutex_);

  cachedhits_ = hits_;

  uint64_t init_amount = hits_.size();

  hits_.erase(std::remove_if(hits_.begin(), hits_.end(),
                             [&KeepValue](const HitInfoT &hit) {
                               return hit.value != KeepValue;
                               ;
                             }),
              hits_.end());
  Log::Info(std::to_string(hits_.size()) + " Hits left. (" +
            std::to_string(init_amount - hits_.size()) + " filtered.)");
}

uint64_t HitList::count() {
  std::scoped_lock<std::mutex> lock(mutex_);
  return hits_.size();
}