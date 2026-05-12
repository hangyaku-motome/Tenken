#pragma once

#include "ContextDisplay.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <atomic>

class HitsW {
private:
  static void InitW();
  static void EndW();
  HitWAction DrawHitTable(const std::vector<HitInfoT> &Hits,
                          const TargetInfoT &TargetInfo);

  bool IsEditing = false;
  bool JustStartedEditing = false;
  float RefreshDuration = 0;
  int64_t selected_row = -1;

  ContextDisplay Context;

public:
  HitWAction CycleW(const std::vector<HitInfoT> &Hits,
                    const TargetInfoT &TargetInfo,
                    std::atomic<float> Progress = -1);
};
