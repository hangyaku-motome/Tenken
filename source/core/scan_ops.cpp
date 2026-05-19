#include "scan_ops.h"
#include "HitList.h"
#include "LogW.h"
#include "Scanner.h"
#include "types.h"
#include "utils.h"

void ScanOp::rescanAllHits(const Scanner &ScannerObj, HitList &Hit, std::atomic<float> &progress,
                           const TargetTypeT TargetType) {
  auto hit_count = Hit.count();

  for (uint64_t i = 0; i < hit_count; ++i) {
    Hit.rescan(ScannerObj, i, TargetType);
    hit_count = Hit.count();
    progress = static_cast<float>(i) / hit_count;
  }
}

std::vector<HitInfoT> ScanOp::startScan(const Scanner &ScannerObj, SessionState &State) {
  std::vector<MapInfoT> Maps = ScannerObj.getMapRegions();
  if (Maps.empty())
    return {};

  Log::Info(std::to_string(Maps.size()) + " map regions found.");

  std::vector<HitInfoT> ReturnHits;

  for (uint64_t i = 0; i < Maps.size(); ++i) {
    State.ScanProgress = static_cast<float>(i) / Maps.size();
    std::vector<uint8_t> Data = ScannerObj.readAdr(Maps[i].start, Maps[i].end - Maps[i].start);
    if (Data.size() != Maps[i].end - Maps[i].start) {
      Maps.erase(Maps.begin() + i);
      continue;
    }

    // for strings, and raw bytes scanning.
    //  function(targetval, rawbytes, std::vector<bool>validTargetBytes = {})
    //  last arg for XX in byte scan.
    dispatchType(State.TargetInfo.TargetType, [&]<typename T> {
      if constexpr (std::is_same_v<T, std::string> ||
                    std::is_same_v<T, std::vector<uint8_t>>) { // string OR byte.
        for (const auto RelativeOffset : searchRawValue(Data, State.TargetInfo.value)) {
          ReturnHits.push_back(
              {.location = Maps[i].start + RelativeOffset,
               .value = State.TargetInfo.value,
               .previous_value = {},
               .bytes_around = findBytesAround(
                   RelativeOffset, Data, static_cast<uint32_t>(State.TargetInfo.value.size()))});
        }
      } else { // for const types.
        T target;
        memcpy(&target, State.TargetInfo.value.data(), sizeof(T));
        for (const auto RelativeOffset : searchValue(Data, target)) {
          ReturnHits.push_back(
              {.location = Maps[i].start + RelativeOffset,
               .value = State.TargetInfo.value,
               .previous_value = {},
               .bytes_around = findBytesAround(
                   RelativeOffset, Data, static_cast<uint32_t>(State.TargetInfo.value.size()))});
        }
      }
    });
  }
  Log::Info(std::to_string(ReturnHits.size()) + " hits found.");
  return ReturnHits;
}