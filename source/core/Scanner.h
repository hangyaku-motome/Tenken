#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include "platform/ProcessOS.h"
#include "PointerList.h"
#include "types.h"

namespace Context {

static constexpr uint8_t bytes_before = 32;
static constexpr uint8_t bytes_after = 32;

};  // namespace Context

// TODO: explicit unattach for ProcessOS and Scanner.
class Scanner {
  static constexpr uint64_t data_region_flag = uint64_t{1} << 63;

  struct PointerData {
    uint64_t points_to;
    uint64_t adr;  // most significant bit holds is_data_region. Need to pack and unpack value properly.

    bool operator<(const PointerData& o) const { return points_to < o.points_to; };
  };

  void buildPointers(const std::vector<PointerData>& pointers,
                     const std::vector<MapInfo>& data_regions,
                     const Pointer::ScanInfo& config,
                     std::ofstream& save_stream,
                     std::array<int64_t, Pointer::max_depth>& stack,
                     uint64_t address,
                     uint8_t depth) const;

  std::filesystem::path makePointerSavePath(const std::string& exec_name) const;
  bool initPointerSaveFile(const std::vector<MapInfo>& data_regions, std::ofstream& save_stream) const;

  std::vector<Pointer::Chain> resolveChains(const std::vector<Pointer::Chain>& chains,
                                            const std::vector<uint64_t>& region_starts,
                                            uint64_t target_address) const;
  bool
  resolveChain(const Pointer::Chain& chain, const std::vector<uint64_t>& region_starts, uint64_t target_address) const;

  std::unique_ptr<ProcessOS::IProcess> proc_ = nullptr;

public:
  void init(int pid) { proc_ = ProcessOS::attach(pid); }

  std::vector<uint8_t> readAdr(uint64_t address, uint64_t read_size) const;
  ReadRegion readAround(uint64_t adr, uint64_t bytes_before, uint64_t bytes_after) const;

  bool writeAdr(const std::vector<uint8_t>& value, uint64_t address) const;

  std::vector<MapInfo> getMapRegions() const;

  Snapshot getSnapshot(const std::vector<MapInfo>& active_regions, std::atomic<float>& progress) const;
  std::vector<HitInfo> filterSnapshot(const Snapshot& old, RelativeStatus keep_type, TargetType target_type) const;

  bool isAttached() const;

  bool findPointerCandidates(const Snapshot& snapshot, const Pointer::InitConfig& config) const;
  void resolvePointerResult(const uint64_t target_address,
                            const std::filesystem::path& save_file_path,
                            PointerList& pointer_list) const;
};
