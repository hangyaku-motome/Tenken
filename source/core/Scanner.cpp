#include "Scanner.h"

#include <imgui.h>
#include <X11/Xdefs.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "Log.h"
#include "Platform.h"
#include "types.h"
#include "utils.h"
#include "version.h"

bool Scanner::isAttached() const {
  if (proc_ == nullptr) return false;
  return proc_->isAttached();
};

bool Scanner::writeAdr(const std::vector<uint8_t>& value, uint64_t address) const {
  if (not isAttached()) return false;
  return proc_->write(value, address);
};

std::vector<uint8_t> Scanner::readAdr(uint64_t address, uint64_t read_size) const {
  if (not isAttached()) return {};
  return proc_->read(address, read_size);
};

// we could maybe std::move the ActiveRegions since we probably won't need them afterwards but...Meh.
//...edit. we kind of DO need it for now. if we want to move it, then we need to make sure on target change and scan
// restart ActiveRegion will be filled in again.
//  Well...Actually it SHOULD be filled in again, since we check for empty but this means...what's the point if we are
//  going to rescan to fill it up again?
//  edit. what was I talking about up there? will look at this later...
//  noo idea
//  I'll have an idea soon enough
//  probably
Snapshot Scanner::getSnapshot(const std::vector<MapInfo>& active_regions, std::atomic<float>& progress) const {
  if (not isAttached()) return {};
  std::vector<MapInfo> maps = active_regions;
  Snapshot snapshot;

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
    memcpy(ptr, data.data(), maps[i].end - maps[i].start);

    snapshot.regions.push_back(Region(maps[i], ptr));
  }
  return snapshot;
}

std::vector<HitInfo>
Scanner::filterSnapshot(const Snapshot& snapshot, RelativeStatus keep_types, TargetType target_types) const {
  if (not isAttached()) return {};
  std::vector<HitInfo> hits;
  RelativeStatus status;

  dispatchType(target_types, [&]<typename T> {
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<uint8_t>>)
      throw std::runtime_error("why is filter snapshot called with string or AOB.");
    else {
      for (uint64_t i = 0; i < snapshot.regions.size(); ++i) {
        auto new_data = proc_->read(snapshot.regions[i].map.start, snapshot.regions[i].size());
        if (new_data.empty()) continue;

        for (uint64_t k = 0; k + sizeof(T) <= snapshot.regions[i].size(); k += sizeof(T)) {
          if (k + sizeof(T) >= new_data.size()) break;

          T new_value;
          T old_value;
          memcpy(&new_value, new_data.data() + k, sizeof(T));
          memcpy(&old_value, snapshot.regions[i].ptr + k, sizeof(T));
          status = tagChange(new_value, old_value);

          if (keep_types == RelativeStatus::changed) {
            if (status != RelativeStatus::increased && status != RelativeStatus::decreased) continue;
          } else if (status != keep_types)
            continue;

          HitInfo hit;
          hit.location = snapshot.regions[i].map.start + k;
          hit.bytes_around = findBytesAround(new_data, k, Context::BytesBefore, Context::BytesAfter + sizeof(T));
          hit.value.resize(sizeof(T));
          memcpy(hit.value.data(), &new_value, sizeof(T));
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

bool Scanner::findPointerCandidates(const Snapshot& snapshot, const Pointer::InitConfig& init_config) const {
  if (not isAttached()) return {};
  std::vector<PointerData> potential_pointers;
  for (uint64_t i = 0; i < snapshot.regions.size(); ++i) {
    uint64_t ptr;
    for (uint64_t k = 0; k + 8 <= snapshot.regions[i].size(); k += sizeof(uint64_t)) {
      memcpy(&ptr, snapshot.regions[i].ptr + k, sizeof(ptr));
      if (ptr < snapshot.regions.front().map.start || ptr > snapshot.regions.back().map.end) continue;
      bool is_data_region = snapshot.regions[i].map.type == MapType::sharedLibData ||
                            snapshot.regions[i].map.type == MapType::mainExecData;
      potential_pointers.push_back(
          PointerData{.points_to = ptr,
                      .adr = (is_data_region ? (k + snapshot.regions[i].map.start) | data_region_flag
                                             : k + snapshot.regions[i].map.start)});
    }
  }
  std::sort(potential_pointers.begin(), potential_pointers.end());

  std::string main_module;
  std::vector<MapInfo> data_regions;
  for (uint64_t i = 0; i < snapshot.regions.size(); ++i) {
    if (snapshot.regions[i].map.type != MapType::sharedLibData && snapshot.regions[i].map.type != MapType::mainExecData)
      continue;
    if (snapshot.regions[i].map.type == MapType::mainExecData) main_module = snapshot.regions[i].map.name;
    data_regions.push_back(snapshot.regions[i].map);
  }
  std::sort(data_regions.begin(), data_regions.end());

  main_module =
      main_module.substr(main_module.find_last_of('/') + 1);  /// suuuuperr wonky but fine for an initial prototype.

  auto save_path = makePointerSavePath(main_module);
  std::ofstream save_stream(save_path, std::ios::binary | std::ios::trunc);
  if (!initPointerSaveFile(data_regions, save_stream, 0)) {
    Log::error("Couldn't initialize pointer save file, tried to save it at " + save_path.string());
    return false;
  };

  std::array<int64_t, Pointer::max_depth> stack{};
  buildPointers(potential_pointers, data_regions, init_config.info, save_stream, stack, init_config.address, 0);

  return true;
}

// TODO: I need to set up some sort of multithreading scan.
void Scanner::buildPointers(const std::vector<PointerData>& pointers,
                            const std::vector<MapInfo>& data_regions,
                            const Pointer::ScanInfo& config,
                            std::ofstream& save_stream,
                            std::array<int64_t, Pointer::max_depth>& stack,
                            uint64_t address,
                            uint8_t depth) const {
  auto it = std::lower_bound(pointers.begin(), pointers.end(), address, [config](const auto& ptr, const uint64_t adr) {
    return ptr.points_to < adr - config.search_before;
  });

  for (; it != pointers.end() && it->points_to <= address + config.search_after; ++it) {
    if ((it->adr & data_region_flag)) {
      // days of debugging just for the problem to be a case of "a - b instead of b - a"
      // absolute cinema.
      stack[depth] = signedDiff(address, it->points_to);
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
      // I removed continue because on further experimenting and thought...what if something from data points to another
      // thing to data? we shouldn't throw those pointers off.
    }
    if (depth + 1 == config.scan_depth) continue;
    stack[depth] = signedDiff(address, it->points_to);
    buildPointers(pointers, data_regions, config, save_stream, stack, it->adr & ~data_region_flag, depth + 1);
  }
}

std::filesystem::path Scanner::makePointerSavePath(const std::string& exec_name) const {
  auto date = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
  std::string date_str = std::format("{:%Y-%m-%d_%H%M%S}", date);
  date_str += ".tptr";

  auto save_path = Platform::getTenkenStatePath() / "Pointer" / exec_name / date_str;
  std::filesystem::create_directories(save_path.parent_path());

  return save_path;
}

// - filter_index (0 for initial scan, +1 for every filter done) uint8_t
bool Scanner::initPointerSaveFile(const std::vector<MapInfo>& data_regions,
                                  std::ofstream& save_stream,
                                  uint8_t filter_index) const {
  save_stream.write(reinterpret_cast<const char*>(&Pointer::magic_bytes), sizeof(Pointer::magic_bytes));
  save_stream.write(reinterpret_cast<const char*>(&PointerResultVersion), sizeof(PointerResultVersion));
  uint8_t entry_size = sizeof(Pointer::Chain);
  save_stream.write(reinterpret_cast<const char*>(&entry_size), sizeof(entry_size));

  auto entry_start_point_seek = save_stream.tellp();  // header size unknown right now.

  save_stream.seekp(sizeof(uint64_t), std::ios::cur);

  save_stream.write(reinterpret_cast<const char*>(&filter_index), sizeof(filter_index));

  for (const auto& reg : data_regions) {
    save_stream.put(static_cast<int8_t>(reg.name.length()));
    save_stream.write(reinterpret_cast<const char*>(reg.name.data()), reg.name.length());
  }

  auto entry_end_point_seek = save_stream.tellp();
  save_stream.seekp(entry_start_point_seek);
  save_stream.write(reinterpret_cast<const char*>(&entry_end_point_seek), sizeof(uint64_t));
  save_stream.seekp(entry_end_point_seek);

  if (!save_stream) {
    Log::error("failed to initalize pointer save file");
    return false;
  };

  Log::info("Pointer file was successfully initialized");
  return true;
}

std::vector<Pointer::Chain> Scanner::resolveChains(const std::vector<Pointer::Chain>& chains,
                                                   const std::vector<uint64_t>& region_starts,
                                                   const uint64_t target_address) const {
  std::vector<Pointer::Chain> new_chains;
  new_chains.reserve(chains.size());

  std::copy_if(chains.begin(), chains.end(), std::back_inserter(new_chains), [&](const Pointer::Chain& c) {
    return resolveChain(c, region_starts, target_address);
  });

  return new_chains;
}

// it works.......
bool Scanner::resolveChain(const Pointer::Chain& chain,
                           const std::vector<uint64_t>& region_starts,
                           const uint64_t target_address) const {
  if (region_starts[chain.module_id] == 0) return false;
  uint64_t ptr;

  ptr = dataToType<uint64_t>(readAdr(region_starts[chain.module_id] + chain.offset_in_module, sizeof(uint64_t)));

  for (int32_t i = 0; i < chain.valid_offsets - 1; ++i) {
    ptr = dataToType<uint64_t>(readAdr(ptr + chain.offsets[i], sizeof(uint64_t)));
    if (ptr == 0) return false;
  }

  if (ptr + chain.offsets[chain.valid_offsets - 1] == target_address) return true;

  return false;
}

// uhh let's hope this all conveniently works the first time and I won't be stuck debugging.
// ^^ disturbingly loud incorrect buzzer
void Scanner::resolvePointerResult(const uint64_t target_address, PointerList& pointer_list) const {
  if (not isAttached()) return;
  std::vector<MapInfo> data_regions;
  std::string main_module_name;
  for (const auto& region : getMapRegions()) {
    if (region.type != MapType::sharedLibData && region.type != MapType::mainExecData) continue;
    if (std::find(pointer_list.data_module_names_.begin(), pointer_list.data_module_names_.end(), region.name) ==
        pointer_list.data_module_names_.end())
      if (region.type == MapType::mainExecData) main_module_name = region.name;
    continue;
    data_regions.push_back(region);
  }
  // sooooooo data_module_names are already sorted by their start address.
  // So I can just sort data_regions and.......that should be good enough!

  std::sort(data_regions.begin(), data_regions.end());

  std::vector<uint64_t> region_starts;
  std::vector<uint32_t> invalid_module_ids;
  for (uint32_t i = 0; i < pointer_list.data_module_names_.size(); ++i) {
    auto it = std::find_if(data_regions.begin(), data_regions.end(), [&](const auto& r) {
      return r.name == pointer_list.data_module_names_[i];
    });

    if (it != data_regions.end())
      region_starts.push_back(it->start);
    else {
      invalid_module_ids.push_back(i);
      region_starts.push_back(0);
    }
    continue;
  }

  std::ofstream save_stream(makePointerSavePath(main_module_name), std::ios::binary | std::ios::trunc);

  // pointer scanning at the start. because it literally just...includes them all, no?
  initPointerSaveFile(data_regions, save_stream, pointer_list.getSaveIndex());

  // not battle tested at alllllllll
  for (uint64_t i = 0; i < pointer_list.total_chains_;) {
    uint64_t process_amount = i + 30000 > pointer_list.total_chains_ ? pointer_list.total_chains_ - i : 30000;
    auto chains = pointer_list.getFromRaw(i, process_amount);

    if (!invalid_module_ids.empty())
      std::erase_if(chains, [&](const auto& c) {
        return std::find(invalid_module_ids.begin(), invalid_module_ids.end(), c.module_id) == invalid_module_ids.end();
      });

    chains = resolveChains(chains, region_starts, target_address);

    if (!invalid_module_ids.empty())
      for (auto& chain : chains) {
        auto it = std::lower_bound(invalid_module_ids.begin(), invalid_module_ids.end(), chain.module_id);
        if (it == invalid_module_ids.end()) it = std::prev(it);
        chain.module_id -= it - invalid_module_ids.begin() + 1;
      }

    if (!chains.empty())  // kinda unnecessary check I guess but whatever
      save_stream.write(reinterpret_cast<const char*>(chains.data()), chains.size() * sizeof(Pointer::Chain));

    i += process_amount;
  }
}

// so it won't try to read a smaller region, it will just try to change it's offset until it can read. should be okay
// for now.
ReadBlock Scanner::readAround(uint64_t adr, uint64_t bytes_before, uint64_t bytes_after) const {
  uint64_t search_start = signedDiff(adr, bytes_before) < 0 ? 0 : adr - bytes_after;

  for (uint32_t i = 0; i < 8; ++i) {
    auto read_bytes = readAdr(search_start, bytes_after + bytes_before);

    if (!read_bytes.empty()) return {.read_bytes = read_bytes, .offset_from_adr = signedDiff(search_start, adr)};
    if (adr > search_start)
      search_start += bytes_before / 4;
    else
      search_start += bytes_after / 4;
  }
  return {};
}
