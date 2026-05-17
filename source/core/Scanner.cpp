#include "Scanner.h"

#include "LogW.h"
#include "platform/ActOS.h"
#include "types.h"
#include "utils.h"
#include <cstdint>
#include <string>
#include <vector>

bool Scanner::writeAdr(uint64_t address,
                       const std::vector<uint8_t> &value) const {
  return proc_->write(address, value);
};
std::vector<uint8_t> Scanner::readAdr(uint64_t address,
                                      uint64_t readSize) const {
  return proc_->read(address, readSize);
};

std::vector<HitInfoT> Scanner::startScan(const std::vector<uint8_t> &targetval,
                                         std::atomic<float> &progress) const {
  std::vector<MapInfoT> Maps = getMapRegions();
  if (Maps.empty())
    return {};

  Log::Info(std::to_string(Maps.size()) + " map regions found.");

  std::vector<HitInfoT> ReturnHits;

  for (uint64_t i = 0; i < Maps.size(); ++i) {
    progress = static_cast<float>(i) / Maps.size();
    std::vector<uint8_t> Data =
        readAdr(Maps[i].start, Maps[i].end - Maps[i].start);
    if (Data.size() != Maps[i].end - Maps[i].start) {
      Maps.erase(Maps.begin() + i);
      continue;
    }
    for (const auto RelativeOffset : searchValue(Data, targetval)) {
      ReturnHits.push_back(
          {.location = Maps[i].start + RelativeOffset,
           .value = targetval,
           .previous_value = {},
           .bytes_around = findBytesAround(
               RelativeOffset, Data, static_cast<uint32_t>(targetval.size()))});
    }
  }
  Log::Info(std::to_string(ReturnHits.size()) + " hits found.");
  return ReturnHits;
}

Snapshot Scanner::StartUnknownValueScan(std::atomic<float> &progress) const {
  std::vector<MapInfoT> Maps = proc_->getRegions();
  std::vector<MappedRegion> regs;

  for (uint64_t i = 0; i < Maps.size(); ++i) {
    progress = static_cast<float>(i) / Maps.size();
    char *ptr = proc_->AllocMMapDisk(Maps[i].end - Maps[i].start);
    if (ptr == nullptr) {
      Log::Error("mmap failed for region " + std::to_string(i) + " will skip.");
      Maps.erase(Maps.begin() + i);
      --i;
      continue;
    }
    auto data = proc_->read(Maps[i].start, Maps[i].end - Maps[i].start);
    if (data.size() != Maps[i].end - Maps[i].start) {
      Log::Error("partial read for region " + std::to_string(i) +
                 " will skip.");
      proc_->UnAllocMMapDisk(reinterpret_cast<uint64_t>(ptr),
                             Maps[i].end - Maps[i].start);
      Maps.erase(Maps.begin() + i);
      --i;
      continue;
    }
    memcpy(ptr, data.data(), Maps[i].end - Maps[i].start);
    regs.push_back({ptr, Maps[i].end - Maps[i].start});
  }
  Snapshot ReturnSnapshot{std::move(regs), std::move(Maps)};
  return ReturnSnapshot;
}

std::vector<HitInfoT> Scanner::FilterSnapshots(const Snapshot &Old,
                                               RelativeStatus KeepType,
                                               TargetTypeT TargetType) const {
  std::vector<HitInfoT> Hits;
  RelativeStatus status;

  dispatchType(TargetType, [&]<typename T> {
    for (uint64_t i = 0; i < Old.maps.size(); ++i) {
      auto new_data = proc_->read(Old.maps[i].start, Old.regions[i].size);
      if (new_data.empty())
        continue;

      for (uint64_t k = 0; k + sizeof(T) <= Old.regions[i].size;
           k += sizeof(T)) {
        if (k + sizeof(T) >= new_data.size())
          break;

        status = compareValues<T>(
            Old.regions[i].ptr + k,
            reinterpret_cast<const char *>(new_data.data() + k));

        if (KeepType == RelativeStatus::CHANGED) {
          if (status != RelativeStatus::INCREASED &&
              status != RelativeStatus::DECREASED)
            continue;
        } else if (status != KeepType)
          continue;

        HitInfoT PushHit;
        PushHit.location = Old.maps[i].start + k;
        PushHit.bytes_around =
            findBytesAround(static_cast<uint32_t>(k), new_data, sizeof(T));
        PushHit.value.assign(PushHit.bytes_around.begin() + BYTES_BEFORE,
                             PushHit.bytes_around.begin() + BYTES_BEFORE +
                                 sizeof(T));
        PushHit.Status = status;
        Hits.push_back(PushHit);
      }
    }
  });
  return Hits;
};