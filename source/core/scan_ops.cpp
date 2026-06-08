#include "scan_ops.h"

#include <cstdint>
#include <type_traits>

#include "HitList.h"
#include "LogW.h"
#include "Scanner.h"
#include "types.h"
#include "utils.h"

void ScanOp::rescanAllHits(const Scanner& ScannerObj,
                           HitList& Hit,
                           std::atomic<float>& progress,
                           const TargetTypeT TargetType) {
  auto hit_count = Hit.count();

  for (uint64_t i = 0; i < hit_count; ++i) {
    Hit.rescan(ScannerObj, i, TargetType);
    hit_count = Hit.count();
    progress = static_cast<float>(i) / hit_count;
  }
}

std::vector<HitInfoT> ScanOp::startScan(const Scanner& ScannerObj, SessionState& State) {
  std::vector<MapInfoT> Maps = ScannerObj.getMapRegions();
  if (Maps.empty()) return {};

  Log::Info(std::to_string(Maps.size()) + " map regions found.");

  std::vector<HitInfoT> ReturnHits;

  for (uint64_t i = 0; i < Maps.size(); ++i) {
    State.ScanProgress = static_cast<float>(i) / Maps.size();
    std::vector<uint8_t> Data = ScannerObj.readAdr(Maps[i].start, Maps[i].end - Maps[i].start);
    if (Data.size() != Maps[i].end - Maps[i].start) {
      Maps.erase(Maps.begin() + i);
      continue;
    }

    dispatchType(State.TargetInfo.TargetType, [&]<typename T> {
      if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
        for (const auto RelativeOffset : searchValue(Data, State.TargetInfo.value, State.TargetInfo.mask.value())) {
          HitInfoT PushHit;
          PushHit.location = Maps[i].start + RelativeOffset;
          PushHit.bytes_around =
              findBytesAround(RelativeOffset, Data, static_cast<uint32_t>(State.TargetInfo.value.size()));
          std::vector<uint8_t> value(PushHit.bytes_around.begin() + BYTES_BEFORE,
                                     PushHit.bytes_around.begin() + BYTES_BEFORE + State.TargetInfo.value.size());
          PushHit.value = value;
          ReturnHits.push_back(PushHit);
        }
      } else {
        T target;
        if constexpr (std::is_same_v<T, std::string>) {
          target.resize(State.TargetInfo.value.size());
          memcpy(target.data(), State.TargetInfo.value.data(), target.size());
        } else
          memcpy(&target, State.TargetInfo.value.data(), sizeof(T));
        for (const auto RelativeOffset : searchValue(Data, target)) {
          HitInfoT PushHit;
          PushHit.location = Maps[i].start + RelativeOffset;
          PushHit.bytes_around =
              findBytesAround(RelativeOffset, Data, static_cast<uint32_t>(State.TargetInfo.value.size()));
          std::vector<uint8_t> value(PushHit.bytes_around.begin() + BYTES_BEFORE,
                                     PushHit.bytes_around.begin() + BYTES_BEFORE + State.TargetInfo.value.size());
          PushHit.value = value;
          ReturnHits.push_back(PushHit);
        }
      }
    });
  }
  Log::Info(std::to_string(ReturnHits.size()) + " hits found.");
  return ReturnHits;
}
