#include "Scanner.h"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "LogW.h"
#include "platform/ActOS.h"
#include "types.h"
#include "utils.h"

bool Scanner::writeAdr(uint64_t address, const std::vector<uint8_t>& value) const {
  if (proc_ == nullptr) return false;
  return proc_->write(address, value);
};

std::vector<uint8_t> Scanner::readAdr(uint64_t address, uint64_t readSize) const {
  if (proc_ == nullptr) return {};
  return proc_->read(address, readSize);
};

Snapshot Scanner::StartUnknownValueScan(std::atomic<float>& progress) const {
  if (proc_ == nullptr) std::runtime_error("start unk scan shouldn't be able to be called while scanner is not init.");
  std::vector<MapInfoT> Maps = proc_->getRegions();
  std::vector<MappedRegion> regs;

  for (uint64_t i = 0; i < Maps.size(); ++i) {
    progress = static_cast<float>(i) / Maps.size();
    char* ptr = proc_->AllocMMapDisk(Maps[i].end - Maps[i].start);
    if (ptr == nullptr) {
      Log::Error("mmap failed for region " + std::to_string(i + 1) + " will skip.");  // still wonky but whatever.
      Maps.erase(Maps.begin() + i);
      --i;
      continue;
    }
    auto data = proc_->read(Maps[i].start, Maps[i].end - Maps[i].start);
    if (data.size() != Maps[i].end - Maps[i].start) {
      Log::Error("partial read for region " + std::to_string(i) + " will skip.");
      proc_->UnAllocMMapDisk(reinterpret_cast<uint64_t>(ptr), Maps[i].end - Maps[i].start);
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

std::vector<HitInfoT>
Scanner::FilterSnapshots(const Snapshot& Old, RelativeStatus KeepType, TargetTypeT TargetType) const {
  std::vector<HitInfoT> Hits;
  RelativeStatus status;

  dispatchType(TargetType, [&]<typename T> {
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<uint8_t>>)
      std::runtime_error("why is filter called with string or AOB.");

    for (uint64_t i = 0; i < Old.maps.size(); ++i) {
      auto new_data = proc_->read(Old.maps[i].start, Old.regions[i].size);
      if (new_data.empty()) continue;

      for (uint64_t k = 0; k + sizeof(T) <= Old.regions[i].size; k += sizeof(T)) {
        if (k + sizeof(T) >= new_data.size()) break;

        T new_value;
        T old_value;
        memcpy(&new_value, new_data.data() + k, sizeof(T));
        memcpy(&old_value, Old.regions[i].ptr + k, sizeof(T));
        status = tagChange(new_value, old_value);

        if (KeepType == RelativeStatus::CHANGED) {
          if (status != RelativeStatus::INCREASED && status != RelativeStatus::DECREASED) continue;
        } else if (status != KeepType)
          continue;

        HitInfoT PushHit;
        PushHit.location = Old.maps[i].start + k;
        PushHit.bytes_around = findBytesAround(static_cast<uint32_t>(k), new_data, sizeof(T));
        PushHit.value.assign(PushHit.bytes_around.begin() + BYTES_BEFORE,
                             PushHit.bytes_around.begin() + BYTES_BEFORE + sizeof(T));
        PushHit.status = status;
        Hits.push_back(PushHit);
      }
    }
  });
  return Hits;
};

std::vector<MapInfoT> Scanner::getMapRegions() const {
  if (proc_ != nullptr) return proc_->getRegions();
  return {};
}
