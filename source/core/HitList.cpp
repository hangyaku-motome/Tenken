#include "HitList.h"

#include <algorithm>

#include "Log.h"
#include "types.h"
#include "utils.h"

void HitList::assignNew(std::vector<HitInfo> new_hits) {
  std::scoped_lock<std::mutex> lock(mutex_);
  hits_ = std::move(new_hits);
}

void HitList::rescan(const Scanner& scanner, uint64_t index, const TargetType target_type) {
  std::scoped_lock<std::mutex> lock(mutex_);
  hits_[index].previous_value = hits_[index].value;

  hits_[index].bytes_around =
      scanner.readAround(hits_[index].location, Context::BytesBefore, Context::BytesAfter + hits_[index].value.size());

  if (hits_[index].bytes_around.read_bytes.size() < hits_[index].value.size() ||
      hits_[index].bytes_around.read_bytes.size() == 0 || hits_[index].bytes_around.offset_from_adr > 0) {
    hits_.erase(hits_.begin() + static_cast<int64_t>(index));
    return;
  }
  hits_[index].value.assign(hits_[index].bytes_around.read_bytes.begin() - hits_[index].bytes_around.offset_from_adr,
                            hits_[index].bytes_around.read_bytes.begin() - hits_[index].bytes_around.offset_from_adr +
                                static_cast<int64_t>(hits_[index].value.size()));

  if (!hits_[index].previous_value.empty())
    dispatchType(target_type, [&]<typename T> {
      hits_[index].status = tagChange(dataToType<T>(hits_[index].value), dataToType<T>(hits_[index].previous_value));
    });
}

void HitList::write(const Scanner& scanner_obj, uint64_t index, const std::vector<uint8_t>& value) {
  std::scoped_lock<std::mutex> lock(mutex_);
  scanner_obj.writeAdr(value, hits_[index].location);
}

void HitList::filter(RelativeStatus keep_type) {
  std::scoped_lock<std::mutex> lock(mutex_);
  RelativeStatus KeepType2 = keep_type;
  RelativeStatus KeepType3 = keep_type;
  if (keep_type == RelativeStatus::changed) {
    KeepType2 = RelativeStatus::increased;
    KeepType3 = RelativeStatus::decreased;
  }

  cached_hits_ = hits_;

  uint64_t init_amount = hits_.size();

  hits_.erase(std::remove_if(hits_.begin(),
                             hits_.end(),
                             [keep_type, KeepType2, KeepType3](const HitInfo& hit) {
                               return hit.status != keep_type && hit.status != KeepType2 && hit.status != KeepType3;
                             }),
              hits_.end());

  Log::info("{} hits left. ({} filtered)", hits_.size(), init_amount - hits_.size());
}

void HitList::filter(const std::vector<uint8_t>& keep_value) {
  std::scoped_lock<std::mutex> lock(mutex_);

  cached_hits_ = hits_;

  uint64_t init_amount = hits_.size();

  hits_.erase(std::remove_if(hits_.begin(),
                             hits_.end(),
                             [&keep_value](const HitInfo& hit) {
                               return hit.value != keep_value;
                               ;
                             }),
              hits_.end());

  Log::info("{} hits left. ({} filtered)", hits_.size(), init_amount - hits_.size());
}

uint64_t HitList::count() {
  std::scoped_lock<std::mutex> lock(mutex_);
  return hits_.size();
}

const std::vector<HitInfo> HitList::getAll() {
  std::scoped_lock<std::mutex> lock(mutex_);
  return hits_;
}

const HitInfo HitList::getIndex(uint64_t index) {
  std::scoped_lock<std::mutex> lock(mutex_);
  return hits_[index];
}

void HitList::reset() {
  std::scoped_lock<std::mutex> lock(mutex_);
  hits_.clear();
}

void HitList::restore_old_hits() {
  std::scoped_lock<std::mutex> lock(mutex_);

  hits_ = std::move(cached_hits_);
}

/// You MUST call unlock before and lock after using this.
const std::vector<HitInfo>& HitList::getRef() { return hits_; }

void HitList::lock() { mutex_.lock(); }

void HitList::unlock() { mutex_.unlock(); }

bool HitList::try_lock() { return mutex_.try_lock(); }
