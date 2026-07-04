#include "Scanner.h"

#include "LogW.h"
#include "types.h"
#include "utils.h"

bool Scanner::writeAdr(uint64_t address, const std::vector<uint8_t>& value) const {
  if (proc_ == nullptr) return false;
  return proc_->write(address, value);
};

std::vector<uint8_t> Scanner::readAdr(uint64_t address, uint64_t read_size) const {
  if (proc_ == nullptr) return {};
  return proc_->read(address, read_size);
};

// we could maybe std::move the ActiveRegions since we probably won't need them afterwards but...Meh.
//...edit. we kind of DO need it for now. if we want to move it, then we need to make sure on target change and scan
// restart ActiveRegion will be filled in again.
//  Well...Actually it SHOULD be filled in again, since we check for empty but this means...what's the point if we are
//  going to rescan to fill it up again?
Snapshot Scanner::getSnapshot(const std::vector<MapInfoT>& active_regions, std::atomic<float>& progress) const {
  if (proc_ == nullptr) return {};
  std::vector<MapInfoT> maps = active_regions;
  std::vector<MappedRegion> regs;

  for (uint64_t i = 0; i < maps.size(); ++i) {
    progress = static_cast<float>(i) / maps.size();
    char* ptr = proc_->allocMMapDisk(maps[i].end - maps[i].start);
    if (ptr == nullptr) {
      Log::error("mmap failed for region " + std::to_string(i + 1) + " will skip.");  // still wonky but whatever.
      maps.erase(maps.begin() + i);
      --i;
      continue;
    }
    auto data = proc_->read(maps[i].start, maps[i].end - maps[i].start);
    if (data.size() != maps[i].end - maps[i].start) {
      Log::error("partial read for region " + std::to_string(i) + " will skip.");
      proc_->unAllocMMapDisk(reinterpret_cast<uint64_t>(ptr), maps[i].end - maps[i].start);
      maps.erase(maps.begin() + i);
      --i;
      continue;
    }
    memcpy(ptr, data.data(), maps[i].end - maps[i].start);
    regs.push_back({ptr, maps[i].end - maps[i].start});
  }
  return {std::move(regs), std::move(maps)}; //idk if these std::move do anything.
}

std::vector<HitInfoT>
Scanner::filterSnapshot(const Snapshot& old, RelativeStatus keep_types, TargetType target_types) const {
  std::vector<HitInfoT> hits;
  RelativeStatus status;

  dispatchType(target_types, [&]<typename T> {
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<uint8_t>>)
      std::runtime_error("why is filter called with string or AOB.");

    for (uint64_t i = 0; i < old.maps.size(); ++i) {
      auto new_data = proc_->read(old.maps[i].start, old.regions[i].size);
      if (new_data.empty()) continue;

      for (uint64_t k = 0; k + sizeof(T) <= old.regions[i].size; k += sizeof(T)) {
        if (k + sizeof(T) >= new_data.size()) break;

        T new_value;
        T old_value;
        memcpy(&new_value, new_data.data() + k, sizeof(T));
        memcpy(&old_value, old.regions[i].ptr + k, sizeof(T));
        status = tagChange(new_value, old_value);

        if (keep_types == RelativeStatus::changed) {
          if (status != RelativeStatus::increased && status != RelativeStatus::decreased) continue;
        } else if (status != keep_types)
          continue;

        HitInfoT hit;
        hit.location = old.maps[i].start + k;
        hit.bytes_around = findBytesAround(static_cast<uint32_t>(k), new_data, sizeof(T));
        hit.value.assign(hit.bytes_around.begin() + bytes_before,
                             hit.bytes_around.begin() + bytes_before + sizeof(T));
        hit.status = status;
        hits.push_back(hit);
      }
    }
  });
  return hits;
};

std::vector<MapInfoT> Scanner::getMapRegions() const {
  if (proc_ != nullptr) return proc_->getRegions();
  return {};
}
