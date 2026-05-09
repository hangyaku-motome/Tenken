#pragma once

#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class HitsW {
private:
  void InitW();
  void EndW();
  Action DrawHitTable(const std::vector<HitInfoT> &Hits,
                      const TargetInfoT &TargetInfo);
  void DrawContextMenu(const HitInfoT Hit);
  bool DrawRefreshAllButton();
  bool DrawRefreshContextButton();
  void AlignButtons();

  bool IsEditing = false;
  bool JustStartedEditing = false;

public:
  int64_t selected_row = -1;
  Action CycleW(const std::vector<HitInfoT> &Hits,
                const TargetInfoT &TargetInfo);
};
