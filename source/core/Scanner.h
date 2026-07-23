#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include "platform/ProcessOS.h"
#include "types.h"

constexpr uint64_t data_region_flag = uint64_t{1} << 63;

struct PointerData {
  uint64_t points_to;
  uint64_t adr;  // most significant bit holds is_data_region. Need to pack and unpack value properly. I did it because a single bool is
                 // going to increase the size of this struct from 16 to 24 which matters for a struct like this that
                 // can be in a vector hundreds of thousands or even more times. Iiiif it ever becomes a problem
                 // changing this is not difficult.

  bool operator<(const PointerData& o) const { return points_to < o.points_to; };
};

class Scanner {
  std::unique_ptr<ProcessOS::IProcess> proc_ = nullptr;

  void buildPointers(const std::vector<PointerData>& pointers,
                     const std::vector<MapInfo>& data_regions,
                     const Pointer::ScanInfo& config,
                     std::ofstream& save_stream,
                     std::array<int64_t, Pointer::max_depth>& stack,
                     uint64_t address,
                     uint8_t depth);

  std::filesystem::path makePointerSavePath(const std::string& exec_name);
  bool initPointerSaveFile(const std::vector<MapInfo>& data_regions, std::ofstream& save_stream);

public:
  void init(int pid) { proc_ = ProcessOS::attach(pid); }

  bool writeAdr(uint64_t address, const std::vector<uint8_t>& value) const;
  std::vector<uint8_t> readAdr(uint64_t address, uint64_t read_size) const;

  std::vector<MapInfo> getMapRegions() const;

  Snapshot getSnapshot(const std::vector<MapInfo>& active_regions, std::atomic<float>& progress) const;

  std::vector<HitInfo> filterSnapshot(const Snapshot& old, RelativeStatus keep_type, TargetType target_type) const;

  bool isAttached() const {
    if (proc_ == nullptr) return false;
    return proc_->isAttached();
  };

  std::string findPointerCandidates(const Snapshot& snapshot, const Pointer::InitConfig& config);
};
