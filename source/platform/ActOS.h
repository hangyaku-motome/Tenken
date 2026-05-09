#pragma once

#include "types.h"
#include <memory>
#include <vector>

namespace ActOS {
class IProcess {
private:
  int pid = 0;

public:
  virtual ~IProcess() = default;
  virtual std::vector<MapInfoT> getRegions() = 0;
  virtual std::vector<uint8_t> read(const uint64_t address,
                                    const uint64_t ReadSize) = 0;
  virtual bool write(const uint64_t address,
                     const std::vector<uint8_t> &value) = 0;
};

std::vector<ProcessInfoT> GetProcTargets();
std::unique_ptr<IProcess> Attach(int pid);

}; // namespace ActOS
