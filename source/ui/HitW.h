#pragma once

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "ContextDisplay.h"
#include "types.h"

class HitW {
private:
  static bool initW();
  static void endW();
  PendingAction drawHitTable(const std::vector<HitInfoT>& hits, const TargetInfoT& target_info);

  bool is_editing_ = false;
  bool just_started_editing_ = false;
  int64_t selected_row_ = -1;

  ContextDisplay context;

public:
  PendingAction cycleW(const std::vector<HitInfoT>& hits, SessionState& state);
};
