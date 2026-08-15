#include "FavouriteList.h"

#include "Scanner.h"
#include "types.h"
#include "utils.h"

void FavouriteList::assignNew(std::vector<FavouriteInfo> new_list) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_ = new_list;
}

void FavouriteList::add(const HitInfo& hit, TargetType target_type) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_.push_back({.bytes_around = hit.bytes_around,
                         .chain = {},
                         .value = hit.value,
                         .previous_value = hit.previous_value,
                         .frozen_value = hit.value,
                         .desc = "",
                         .location = hit.location,
                         .type = target_type});
}

void FavouriteList::add(const Pointer::PrettyChain& chain, uint64_t adr, TargetType target_type, uint8_t target_size) {
  std::scoped_lock<std::mutex> lock(mutex_);

  std::vector<uint8_t> value(target_size);
  favourites_.push_back({.bytes_around = {},
                         .chain = chain,
                         .value = value,
                         .previous_value = {},
                         .frozen_value = {},
                         .desc = "",
                         .location = adr,
                         .type = target_type});
}

void FavouriteList::remove(uint64_t index) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_.erase(favourites_.begin() + static_cast<int64_t>(index));
}

void FavouriteList::setFreeze(uint64_t index, bool set_to) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_[index].frozen = set_to;
}

void FavouriteList::setDesc(uint64_t index, std::string set_to) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_[index].desc = set_to;
}

void FavouriteList::rescanNoLock(const Scanner& scanner, uint64_t index) {
  favourites_[index].previous_value = favourites_[index].value;

  favourites_[index].bytes_around = scanner.readAround(
      favourites_[index].location, Context::BytesBefore, Context::BytesAfter + favourites_[index].value.size());

  if (favourites_[index].bytes_around.read_bytes.size() < favourites_[index].value.size() ||
      favourites_[index].bytes_around.read_bytes.size() == 0 || favourites_[index].bytes_around.offset_from_adr > 0) {
    favourites_.erase(favourites_.begin() + static_cast<int64_t>(index));
    return;
  }

  favourites_[index].value.assign(
      favourites_[index].bytes_around.read_bytes.begin() - favourites_[index].bytes_around.offset_from_adr,
      favourites_[index].bytes_around.read_bytes.begin() - favourites_[index].bytes_around.offset_from_adr +
          static_cast<int64_t>(favourites_[index].value.size()));

  if (!favourites_[index].previous_value.empty())
    dispatchType(favourites_[index].type, [&]<typename T> {
      favourites_[index].status =
          tagChange(dataToType<T>(favourites_[index].value), dataToType<T>(favourites_[index].previous_value));
    });
}

void FavouriteList::rescan(const Scanner& scanner, uint64_t index) {
  std::scoped_lock<std::mutex> lock(mutex_);
  rescanNoLock(scanner, index);
}

void FavouriteList::rescanAll(const Scanner& scanner) {
  std::scoped_lock<std::mutex> lock(mutex_);

  for (uint64_t i = 0; i < favourites_.size(); ++i) rescanNoLock(scanner, i);
}

void FavouriteList::write(const Scanner& scanner, uint64_t index, const std::vector<uint8_t>& value) {
  std::scoped_lock<std::mutex> lock(mutex_);
  scanner.writeAdr(value, favourites_[index].location);
}

void FavouriteList::setFreezeVal(uint64_t index, const std::vector<uint8_t>& set_to) {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_[index].frozen_value = set_to;
}

void FavouriteList::startFreezeThread(const Scanner& scanner) {
  if (freeze_thread_.joinable()) freeze_thread_.join();
  freeze_running_ = true;
  freeze_thread_ = std::thread([&]() {
    while (freeze_running_) {
      {
        std::scoped_lock<std::mutex> lock(mutex_);
        for (const auto& favourite : favourites_)
          if (favourite.frozen) scanner.writeAdr(favourite.frozen_value, favourite.location);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
  });
}

std::vector<FavouriteInfo> FavouriteList::getAll() {
  std::scoped_lock<std::mutex> lock(mutex_);
  return favourites_;
}

void FavouriteList::endFreezeThread() {
  freeze_running_ = false;
  if (freeze_thread_.joinable()) freeze_thread_.join();
}

void FavouriteList::reset() {
  std::scoped_lock<std::mutex> lock(mutex_);
  favourites_.clear();
  endFreezeThread();
}

// you MUST MUST call lock() before this and unlock() after this.
const std::vector<FavouriteInfo>& FavouriteList::getRef() { return favourites_; }

void FavouriteList::lock() { mutex_.lock(); }

void FavouriteList::unlock() { mutex_.unlock(); }

bool FavouriteList::try_lock() { return mutex_.try_lock(); }
