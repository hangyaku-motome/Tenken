#pragma once

#include "platform/ActOS.h"
#include "types.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <memory>
#include <vector>

class Scanner {

private:
  std::unique_ptr<ActOS::IProcess> proc_ = nullptr;
  std::vector<uint32_t> SearchValue(const std::vector<uint8_t> &Data,
                                    const std::vector<uint8_t> &TargetData);

  const std::vector<uint8_t> FindBytesAround(const uint32_t offset,
                                             const std::vector<uint8_t> &data,
                                             const uint32_t TargetSize);

public:
  Scanner() {};

  std::vector<HitInfoT> Hits;

  void Start(int pid);

  void StartScan(const TargetInfoT &TargetInfo);
};
