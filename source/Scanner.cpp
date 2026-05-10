#include "Scanner.h"

#include "LogW.h"
#include "platform/ActOS.h"
#include "types.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>

void Scanner::Init(int pid) { proc_ = ActOS::Attach(pid); }

// I noticed....we are JUST using TargetInfo.value meaning we can just give this
// a vector<uint8_t>. I'll hold off on this because I am already rewriting so
// much based on this and that right now.
void Scanner::StartScan(const TargetInfoT &TargetInfo) {
  Hits.clear();
  std::vector<MapInfoT> Maps = proc_->getRegions();
  if (Maps.empty())
    return;

  TotalLoad = Maps.size();
  CurrentProgress = 0;
  IsScanning = true;

  Log::Info(std::to_string(Maps.size()) + " map regions found.");

  for (const auto &Map : Maps) {
    ++CurrentProgress;

    std::vector<uint8_t> Data = proc_->read(Map.start, Map.end - Map.start);

    for (auto RelativeOffset : SearchValue(Data, TargetInfo.value)) {
      Hits.push_back(
          {TargetInfo.value,
           {},
           Map.start + RelativeOffset,
           FindBytesAround(RelativeOffset, Data, TargetInfo.value.size())});
    }
  }
  IsScanning = false;
  Log::Info(std::to_string(Hits.size()) + " hits found.");
}

std::vector<uint32_t>
Scanner::SearchValue(const std::vector<uint8_t> &Data,
                     const std::vector<uint8_t> &TargetData) {

  std::vector<uint32_t> FoundOffsets;
  uint32_t TargetSize = TargetData.size();

  for (uint32_t i = 0; i + TargetSize <= Data.size(); ++i) {
    if (!memcmp(&Data[i], TargetData.data(), TargetSize)) {
      FoundOffsets.push_back(i);
    }
  }
  return FoundOffsets;
}

const std::vector<uint8_t>
Scanner::FindBytesAround(const uint32_t offset,
                         const std::vector<uint8_t> &data,
                         const uint32_t Size) {
  uint64_t START = offset < 32 ? 0 : offset - 32;
  uint64_t END =
      offset + 32 + Size > data.size() ? data.size() : offset + 32 + Size;

  std::vector<uint8_t> bytes(END - START);

  memcpy(bytes.data(), &data[START], END - START);
  return bytes;
}

// we should also add something that also rescans maps and deals with hits from
// there...at some point.

// oh and a way to compare old and new bytes around? I'd also need to add that
// to hitinfo struct.

template <typename T> void Scanner::RescanEntryData(T &entry) {
  entry.previous_value = entry.value;

  entry.bytes_around =
      proc_->read(entry.location - 32, entry.value.size() + 64);
  memcpy(entry.value.data(), entry.bytes_around.data() + 32,
         entry.value.size());
  return;
}

template <typename T, typename entry_type>
RelativeStatus Scanner::CompareValues(const entry_type &entry) {
  T newval{};
  T oldval{};

  static_assert(std::is_arithmetic_v<T>,
                "CompareValues only works with numeric types");

  memcpy(&newval, entry.value.data(), sizeof(newval));
  memcpy(&oldval, entry.previous_value.data(), sizeof(oldval));

  if (newval == oldval)
    return RelativeStatus::UNCHANGED;
  if (newval > oldval)
    return RelativeStatus::INCREASED;
  if (newval < oldval)
    return RelativeStatus::DECREASED;

  return RelativeStatus::CHANGED;
}

template <typename T>
void Scanner::TagEntryChange(T &entry, const TargetTypeT TargetType) {
  if (TargetType == TargetTypeT::Invalid) {
    return;
  }
  if (TargetType == TargetTypeT::String) {
    if (!memcmp(entry.value.data(), entry.previous_value.data(),
                entry.value.size()))
      entry.Status = RelativeStatus::CHANGED;
    else
      entry.Status = RelativeStatus::UNCHANGED;

    return;
  }

  RelativeStatus Relativestatus = RelativeStatus::UNSET;

  switch (TargetType) {
  case TargetTypeT::Int8:
    Relativestatus = CompareValues<int8_t>(entry);
    break;
  case TargetTypeT::Int16:
    Relativestatus = CompareValues<int16_t>(entry);
    break;
  case TargetTypeT::Int32:
    Relativestatus = CompareValues<int32_t>(entry);
    break;
  case TargetTypeT::Int64:
    Relativestatus = CompareValues<int64_t>(entry);
    break;
  case TargetTypeT::uInt8:
    Relativestatus = CompareValues<uint8_t>(entry);
    break;
  case TargetTypeT::uInt16:
    Relativestatus = CompareValues<uint16_t>(entry);
    break;
  case TargetTypeT::uInt32:
    Relativestatus = CompareValues<uint32_t>(entry);
    break;
  case TargetTypeT::uInt64:
    Relativestatus = CompareValues<uint64_t>(entry);
    break;
  case TargetTypeT::Float:
    Relativestatus = CompareValues<float>(entry);
    break;
  case TargetTypeT::Double:
    Relativestatus = CompareValues<double>(entry);
    break;
  default:
    Log::Error("In TagHitChange why is TargetType invalid?");
    break;
  }

  entry.Status = Relativestatus;
}

void Scanner::FilterHit(const RelativeStatus Keep1) {
  RelativeStatus Keep2 = Keep1;
  RelativeStatus Keep3 = Keep1;
  if (Keep1 == RelativeStatus::CHANGED) {
    Keep2 = RelativeStatus::INCREASED;
    Keep3 = RelativeStatus::DECREASED;
  }

  Hits.erase(std::remove_if(Hits.begin(), Hits.end(),
                            [Keep1, Keep2, Keep3](const HitInfoT &hit) {
                              return hit.Status != Keep1 &&
                                     hit.Status != Keep2 && hit.Status != Keep3;
                            }),
             Hits.end());
}

void Scanner::FilterHit(const std::vector<uint8_t> &keepValue) {

  Hits.erase(std::remove_if(Hits.begin(), Hits.end(),
                            [keepValue](const HitInfoT &hit) {
                              return memcmp(hit.value.data(), keepValue.data(),
                                            keepValue.size()) != 0;
                            }),
             Hits.end());
}

template <typename T>
void Scanner::WriteEntryAdr(const T entry, const std::vector<uint8_t> &value) {
  bool Success = proc_->write(entry.location, value);

  if (!Success) {
    Log::Error("Write to address " + std::to_string(entry.location) +
               " failed!");
  }
}

void Scanner::AddToFavourite(const uint64_t index) { // mutex
  std::scoped_lock<std::mutex> lock(FavouriteMutex);

  FavouriteInfoT PushFavourite;
  PushFavourite.bytes_around = Hits[index].bytes_around;
  PushFavourite.location = Hits[index].location;
  PushFavourite.value = Hits[index].value;
  PushFavourite.frozen_value = PushFavourite.value;
  Favourites.push_back(PushFavourite);
}

template <typename T>
void Scanner::RescanEntry(T &entry, const TargetTypeT &TargetType) {
  RescanEntryData(entry);
  TagEntryChange(entry, TargetType);
}

void Scanner::RescanHit(const uint64_t index, const TargetTypeT &TargetType) {
  RescanEntry(Hits[index], TargetType);
}
void Scanner::RescanFavourite(const uint64_t index,
                              const TargetTypeT &TargetType) { // mutex
  std::scoped_lock<std::mutex> lock(FavouriteMutex);

  Favourites[index].previous_bytes_around = Favourites[index].bytes_around;
  RescanEntry(Favourites[index], TargetType);
}

void Scanner::WriteFavourite(const uint64_t index,
                             const std::vector<uint8_t> &value) {
  std::scoped_lock<std::mutex> lock(FavouriteMutex);

  Favourites[index].frozen_value = value;
  WriteEntryAdr(Favourites[index], value);
}

void Scanner::WriteHit(const uint64_t index,
                       const std::vector<uint8_t> &value) {
  WriteEntryAdr(Hits[index], value);
}

void Scanner::StartFreezeThread() {
  FreezeRunning = true;
  FreezeThread = std::thread([this]() { WriteFreezeValues(); });
}
void Scanner::WriteFreezeValues() {
  while (FreezeRunning) {
    {
      std::scoped_lock<std::mutex> lock(FavouriteMutex);
      for (uint64_t i = 0; i < Favourites.size(); ++i)
        if (Favourites[i].Frozen)
          proc_->write(Favourites[i].location, Favourites[i].frozen_value);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }
}
void Scanner::EndFreezeThread() {
  FreezeRunning = false;
  if (FreezeThread.joinable())
    FreezeThread.join();
}