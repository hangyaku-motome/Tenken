#pragma once

#include "ContextDisplay.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class HitsW {
private:
  static bool InitW();
  static void EndW();
  PendingAction DrawHitTable(const std::vector<HitInfoT> &Hits,
                             const TargetInfoT &TargetInfo);

  bool IsEditing = false;
  bool JustStartedEditing = false;
  int64_t selected_row = -1;

  ContextDisplay Context;

public:
  PendingAction CycleW(const std::vector<HitInfoT> &Hits, SessionState &State);
};
