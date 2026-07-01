#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "types.h"

namespace ProcessOS {
class IProcess {
public:
  virtual ~IProcess() = default;
  virtual std::vector<MapInfoT> getRegions() = 0;
  virtual std::vector<uint8_t> read(uint64_t address, uint64_t ReadSize) = 0;
  virtual bool write(uint64_t address, const std::vector<uint8_t>& value) = 0;
  virtual char* allocMMapDisk(uint64_t size) = 0;
  virtual void unAllocMMapDisk(uint64_t address, uint64_t size) = 0;
};

std::vector<ProcessInfoT> getProcTargets(); // unsure if I should move this to Platform or not.
std::unique_ptr<IProcess> attach(int pid);

};  // namespace ActOS
