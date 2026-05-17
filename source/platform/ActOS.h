#pragma once

#include "types.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace ActOS {
class IProcess {
public:
  virtual ~IProcess() = default;
  virtual std::vector<MapInfoT> getRegions() = 0;
  virtual std::vector<uint8_t> read(uint64_t address, uint64_t ReadSize) = 0;
  virtual bool write(uint64_t address, const std::vector<uint8_t> &value) = 0;
  virtual char *AllocMMapDisk(uint64_t size) = 0;
  virtual void UnAllocMMapDisk(uint64_t address, uint64_t size) = 0;
};

std::vector<ProcessInfoT> GetProcTargets();
std::unique_ptr<IProcess> Attach(int pid);

}; // namespace ActOS
