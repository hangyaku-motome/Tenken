#pragma once

#include <cstdint>
#include <vector>

#include "platform/ProcessOS.h"
#include "types.h"

class Scanner {
  std::unique_ptr<ProcessOS::IProcess> proc_ = nullptr;

public:
  void init(int pid) { proc_ = ProcessOS::attach(pid); }

  bool writeAdr(uint64_t address, const std::vector<uint8_t>& value) const;
  std::vector<uint8_t> readAdr(uint64_t address, uint64_t readSize) const;

  std::vector<MapInfoT> getMapRegions() const;

  Snapshot StartUnknownValueScan(std::atomic<float>& progress) const;

  std::vector<HitInfoT> FilterSnapshots(const Snapshot& Old, RelativeStatus KeepType, TargetTypeT TargetType) const;
};
