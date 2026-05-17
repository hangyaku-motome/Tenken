#pragma once

#include "platform/ActOS.h"
#include "types.h"
#include <cstdint>
#include <vector>

class Scanner {
  std::unique_ptr<ActOS::IProcess> proc_ = nullptr;

public:
  void init(int pid) { proc_ = ActOS::Attach(pid); }

  bool writeAdr(uint64_t address, const std::vector<uint8_t> &value) const;
  std::vector<uint8_t> readAdr(uint64_t address, uint64_t readSize) const;

  std::vector<MapInfoT> getMapRegions() const { return proc_->getRegions(); }

  std::vector<HitInfoT> startScan(const std::vector<uint8_t> &targetval,
                                  std::atomic<float> &progress) const;

  Snapshot StartUnknownValueScan(std::atomic<float> &progress) const;

  std::vector<HitInfoT> FilterSnapshots(const Snapshot &Old,
                                        RelativeStatus KeepType,
                                        TargetTypeT TargetType) const;
};