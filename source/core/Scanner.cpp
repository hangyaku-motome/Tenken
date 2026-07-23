#include "Scanner.h"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

#include "LogW.h"
#include "Platform.h"
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
//  edit. what was I talking about up there? will look at this later...
Snapshot Scanner::getSnapshot(const std::vector<MapInfo>& active_regions, std::atomic<float>& progress) const {
  if (proc_ == nullptr) return {};
  std::vector<MapInfo> maps = active_regions;
  Snapshot return_snapshot;

  std::cout << maps.size() << "\n";
  for (uint64_t i = 0; i < maps.size(); ++i) {
    progress = static_cast<float>(i) / static_cast<float>(maps.size());
    char* ptr = proc_->allocMMapDisk(maps[i].end - maps[i].start);
    if (ptr == nullptr) {
      Log::error("mmap failed for region " + std::to_string(i + 1) + " will skip.");  // still wonky but whatever.
      maps.erase(maps.begin() + static_cast<int64_t>(i));
      --i;
      continue;
    }
    auto data = proc_->read(maps[i].start, maps[i].end - maps[i].start);
    if (data.size() != maps[i].end - maps[i].start) {
      Log::error("partial read for region " + std::to_string(i) + " will skip.");
      proc_->unAllocMMapDisk(reinterpret_cast<uint64_t>(ptr), maps[i].end - maps[i].start);
      maps.erase(maps.begin() + static_cast<int64_t>(i));
      --i;
      continue;
    }
    memcpy(ptr, data.data(), maps[i].end - maps[i].start);  // SIGBUS?? Why?

    return_snapshot.regions.push_back(
        Region{.mapped_region = MappedRegion(ptr, maps[i].end - maps[i].start), .map = maps[i]});
  }
  return return_snapshot;
}

std::vector<HitInfo>
Scanner::filterSnapshot(const Snapshot& snapshot, RelativeStatus keep_types, TargetType target_types) const {
  std::vector<HitInfo> hits;
  RelativeStatus status;

  dispatchType(target_types, [&]<typename T> {
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<uint8_t>>)
      throw std::runtime_error(
          "why is filter called with string or AOB.");  // did I really forget to add throw here lol.
    else {
      for (uint64_t i = 0; i < snapshot.regions.size(); ++i) {
        auto new_data = proc_->read(snapshot.regions[i].map.start, snapshot.regions[i].mapped_region.size);
        if (new_data.empty()) continue;

        for (uint64_t k = 0; k + sizeof(T) <= snapshot.regions[i].mapped_region.size; k += sizeof(T)) {
          if (k + sizeof(T) >= new_data.size()) break;

          T new_value;
          T old_value;
          memcpy(&new_value, new_data.data() + k, sizeof(T));
          memcpy(&old_value, snapshot.regions[i].mapped_region.ptr + k, sizeof(T));
          status = tagChange(new_value, old_value);

          if (keep_types == RelativeStatus::changed) {
            if (status != RelativeStatus::increased && status != RelativeStatus::decreased) continue;
          } else if (status != keep_types)
            continue;

          HitInfo hit;
          hit.location = snapshot.regions[i].map.start + k;
          hit.bytes_around = findBytesAround(static_cast<uint32_t>(k), new_data, sizeof(T));
          hit.value.assign(hit.bytes_around.begin() + bytes_before,
                           hit.bytes_around.begin() + bytes_before + sizeof(T));
          hit.status = status;
          hits.push_back(hit);
        }
      }
    }
  });
  return hits;
};

std::vector<MapInfo> Scanner::getMapRegions() const {
  if (proc_ != nullptr) return proc_->getRegions();
  return {};
}

/// also, heaptrack thing or whatever. try it out.

// returns save
std::string Scanner::findPointerCandidates(const Snapshot& snapshot, const Pointer::InitConfig& init_config) {
  printf("trying to make find pointer candidates..\n");
  std::vector<PointerData> potential_pointers;
  for (uint64_t i = 0; i < snapshot.regions.size(); ++i) {
    uint64_t ptr;
    for (uint64_t k = 0; k + 8 <= snapshot.regions[i].mapped_region.size; k += Pointer::size) {
      memcpy(&ptr, snapshot.regions[i].mapped_region.ptr + k, sizeof(ptr));
      if (ptr < snapshot.regions.front().map.start || ptr > snapshot.regions.back().map.end) continue;
      bool is_data_region = snapshot.regions[i].map.type == MapType::sharedLibData ||
                            snapshot.regions[i].map.type == MapType::mainExecData;
      potential_pointers.push_back(
          PointerData{.points_to = ptr,
                      .adr = (is_data_region ? (k + snapshot.regions[i].map.start) | data_region_flag
                                             : k + snapshot.regions[i].map.start)});

      if (is_data_region) printf("a pointer in data! %lx\n", potential_pointers.back().adr & ~data_region_flag);
    }
  }
  std::sort(potential_pointers.begin(), potential_pointers.end());
  std::cout << "\n\n" << potential_pointers.size() << " potential pointer count\n\n";

  std::string main_module;
  std::vector<MapInfo> data_regions;
  for (uint64_t i = 0; i < snapshot.regions.size(); ++i) {
    if (snapshot.regions[i].map.type != MapType::sharedLibData && snapshot.regions[i].map.type != MapType::mainExecData)
      continue;
    if (snapshot.regions[i].map.type == MapType::mainExecData) main_module = snapshot.regions[i].map.name;
    data_regions.push_back(snapshot.regions[i].map);
  }
  std::sort(data_regions.begin(), data_regions.end());
  std::cout << "\n\n" << data_regions.size() << " data regions count\n\n";

  main_module =
      main_module.substr(main_module.find_last_of('/') + 1);  /// suuuuperr wonky but fine for an initial prototype.

  auto save_path = makePointerSavePath(main_module);
  std::ofstream save_stream(save_path, std::ios::binary | std::ios::trunc);
  if (!save_stream) return {};
  if (!initPointerSaveFile(data_regions, save_stream)) {
    Log::error("Couldn't initialize pointer save file, tried to save it at " + save_path.string());
    return {};
  };

  std::array<int64_t, Pointer::max_depth> stack;
  buildPointers(potential_pointers, data_regions, init_config.info, save_stream, stack, init_config.address, 0);

  printf("finished\n");
  return save_path;
}

void Scanner::buildPointers(const std::vector<PointerData>& pointers,
                            const std::vector<MapInfo>& data_regions,
                            const Pointer::ScanInfo& config,
                            std::ofstream& save_stream,
                            std::array<int64_t, Pointer::max_depth>& stack,
                            uint64_t address,
                            uint8_t depth) {
  auto it = std::lower_bound(pointers.begin(), pointers.end(), address, [config](const auto& ptr, const uint64_t adr) {
    return ptr.points_to < adr - config.search_before;
  });

  for (; it != pointers.end() && it->points_to <= address + config.search_after; ++it) {
    if ((it->adr & data_region_flag)) {
      stack[depth] = signedDiff(it->adr & ~data_region_flag, address);
      auto reg_it = std::upper_bound(data_regions.begin(),
                                     data_regions.end(),
                                     it->adr & ~data_region_flag,
                                     [](const uint64_t v, const auto& r) { return v < r.start; });
      --reg_it;
      Pointer::Chain chain = {.offsets = stack,
                              .offset_in_module = (it->adr & ~data_region_flag) - reg_it->start,
                              .module_id = static_cast<uint32_t>(std::distance(data_regions.begin(), reg_it)),
                              .valid_offsets = static_cast<uint8_t>(depth + 1)};
      save_stream.write(reinterpret_cast<const char*>(&chain), sizeof(Pointer::Chain));
      continue;
    }
    if (depth == config.scan_depth) continue;
    stack[depth] = signedDiff(it->adr & ~data_region_flag, address);
    buildPointers(pointers, data_regions, config, save_stream, stack, it->adr & ~data_region_flag, depth + 1);
  }
}

std::filesystem::path Scanner::makePointerSavePath(const std::string& exec_name) {
  printf("trying to make pointer save pathh..\n");
  auto date = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
  std::string date_str = std::format("{:%Y-%m-%d_%H%M%S}", date);
  date_str += ".tpt";

  auto save_path = Platform::getTenkenStatePath() / "Pointer" / exec_name / date_str;
  std::filesystem::create_directories(save_path.parent_path());
  std::cout << save_path << " that is save path\n";

  return save_path;
}

bool Scanner::initPointerSaveFile(const std::vector<MapInfo>& data_regions, std::ofstream& save_stream) {
  try {
    save_stream.exceptions(std::ios::failbit | std::ios::badbit);
    save_stream.write(reinterpret_cast<const char*>(&Pointer::magic_bytes), sizeof(Pointer::magic_bytes));
    save_stream.write(reinterpret_cast<const char*>(&Pointer::file_version), sizeof(Pointer::file_version));
    uint8_t entry_size = sizeof(Pointer::Chain);
    save_stream.write(reinterpret_cast<const char*>(&entry_size), sizeof(entry_size));

    auto entry_start_point_seek = save_stream.tellp();  // header size unknown right now.

    save_stream.seekp(sizeof(uint64_t), std::ios::cur);

    for (const auto& reg : data_regions) {
      save_stream.put(static_cast<int8_t>(reg.name.length()));
      save_stream.write(reinterpret_cast<const char*>(reg.name.data()), reg.name.length());
    }

    auto entry_end_point_seek = save_stream.tellp();
    save_stream.seekp(entry_start_point_seek);
    save_stream.write(reinterpret_cast<const char*>(&entry_end_point_seek), sizeof(uint64_t));
    save_stream.seekp(entry_end_point_seek);

  } catch (...) {
    return false;
  }

  Log::info("Pointer file was successfully initialized");
  return true;
}
