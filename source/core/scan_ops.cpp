#include "scan_ops.h"

#include <cstdint>
#include <type_traits>

#include "HitList.h"
#include "Log.h"
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
    progress = static_cast<float>(i) / static_cast<float>(hit_count);
  }
}

// here too...won't std::move for now.
// ?
// I guess I was refering to active_region? yeah don't eat it up.
std::vector<HitInfo> ScanOp::startScan(const Scanner& scanner,
                                       const TargetInfo& target_info,
                                       std::atomic<float>& scan_progress,
                                       const std::vector<MapInfo>& active_region) {
  std::vector<MapInfo> maps = active_region;
  if (maps.empty()) return {};

  std::vector<HitInfo> hits;

  for (uint64_t i = 0; i < maps.size(); ++i) {
    scan_progress = static_cast<float>(i) / static_cast<float>(maps.size());
    std::vector<uint8_t> data = scanner.readAdr(maps[i].start, maps[i].end - maps[i].start);
    if (data.size() != maps[i].end - maps[i].start) {
      maps.erase(maps.begin() + static_cast<int64_t>(i));
      continue;
    }

    dispatchType(target_info.target_type, [&]<typename T> {
      if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
        for (const auto rel_offset : searchValue(data, target_info.value, target_info.mask.value())) {
          HitInfo push_hit;
          push_hit.location = maps[i].start + rel_offset;
          push_hit.value = target_info.value;  // reallllyy didn't gotta make it all that complicated like before
          push_hit.bytes_around =
              findBytesAround(data, rel_offset, Context::BytesBefore, Context::BytesAfter + target_info.value.size());
          hits.push_back(push_hit);
        }
      } else {
        T target;
        if constexpr (std::is_same_v<T, std::string>) {
          target.resize(target_info.value.size());
          memcpy(target.data(), target_info.value.data(), target.size());
        } else
          memcpy(&target, target_info.value.data(), sizeof(T));
        for (const auto rel_offset : searchValue(data, target)) {
          HitInfo hit;
          hit.location = maps[i].start + rel_offset;
          hit.value = target_info.value;
          hit.bytes_around =
              findBytesAround(data, rel_offset, Context::BytesBefore, Context::BytesAfter + target_info.value.size());
          hits.push_back(hit);
        }
      }
    });
  }
  Log::info("{} hits found.", hits.size());
  return hits;
}
