#pragma once

#include "display.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <atomic>

// maybe make a single "DrawContext" function that is in display and FavouriteW
// and HitsW share.
class HitsW {
private:
  void InitW();
  void EndW();
  Action DrawHitTable(const std::vector<HitInfoT> &Hits,
                      const TargetInfoT &TargetInfo);

  bool IsEditing = false;
  bool JustStartedEditing = false;

  float RefreshDuration = 0;
  ContextDisplay Context;

public:
  int64_t selected_row = -1;
  Action CycleW(const std::vector<HitInfoT> &Hits,
                const TargetInfoT &TargetInfo,
                std::atomic<float> Progress = -1);
};
