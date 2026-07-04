#include "scan_ops.h"

#include <cstdint>
#include <type_traits>

#include "HitList.h"
#include "LogW.h"
#include "Scanner.h"
#include "types.h"
#include "utils.h"

void ScanOp::rescanAllHits(const Scanner& scanner,
                           HitList& hit,
                           std::atomic<float>& progress,
                           const TargetType target_type) {
  auto hit_count = hit.count();

  for (uint64_t i = 0; i < hit_count; ++i) {
    hit.rescan(scanner, i, target_type);
    hit_count = hit.count();
    progress = static_cast<float>(i) / hit_count;
  }
}

// here too...won't std::move for now.
std::vector<HitInfoT> ScanOp::startScan(const Scanner& scanner, const TargetInfoT& target_info, std::atomic<float> & scan_progress, const std::vector<MapInfoT>& active_region) {
  std::vector<MapInfoT> maps = active_region;
  if (maps.empty()) return {};

  std::vector<HitInfoT> return_hits;

  for (uint64_t i = 0; i < maps.size(); ++i) {
    scan_progress = static_cast<float>(i) / maps.size();
    std::vector<uint8_t> data = scanner.readAdr(maps[i].start, maps[i].end - maps[i].start);
    if (data.size() != maps[i].end - maps[i].start) {
      maps.erase(maps.begin() + i);
      continue;
    }

    dispatchType(target_info.target_type, [&]<typename T> {
      if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
        for (const auto rel_offset : searchValue(data, target_info.value, target_info.mask.value())) {
          HitInfoT PushHit;
          PushHit.location = maps[i].start + rel_offset;
          PushHit.bytes_around =
              findBytesAround(rel_offset, data, static_cast<uint32_t>(target_info.value.size()));
          std::vector<uint8_t> value(PushHit.bytes_around.begin() + bytes_before,
                                     PushHit.bytes_around.begin() + bytes_before + target_info.value.size());
          PushHit.value = value;
          return_hits.push_back(PushHit);
        }
      } else {
        T target;
        if constexpr (std::is_same_v<T, std::string>) {
          target.resize(target_info.value.size());
          memcpy(target.data(), target_info.value.data(), target.size());
        } else
          memcpy(&target, target_info.value.data(), sizeof(T));
        for (const auto rel_offset : searchValue(data, target)) {
          HitInfoT PushHit;
          PushHit.location = maps[i].start + rel_offset;
          PushHit.bytes_around =
              findBytesAround(rel_offset, data, static_cast<uint32_t>(target_info.value.size()));
          std::vector<uint8_t> value(PushHit.bytes_around.begin() + bytes_before,
                                     PushHit.bytes_around.begin() + bytes_before + target_info.value.size());
          PushHit.value = value;
          return_hits.push_back(PushHit);
        }
      }
    });
  }
  Log::info(std::to_string(return_hits.size()) + " hits found.");
  return return_hits;
}
