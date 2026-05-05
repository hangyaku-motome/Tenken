#pragma once

#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class HitsW {
private:
  void InitW();
  void EndW();
  void DrawHitTable(const std::vector<HitInfoT> &Hits,
                    const TargetInfoT &TargetInfo);
  void DrawContextMenu(const HitInfoT Hit);
  std::string DrawRefreshAllButton();
  std::string DrawRefreshContextButton();
  void AlignButtons();

public:
  int64_t selected_row = -1;
  WindowInfoT Window;
  std::string CycleW(const std::vector<HitInfoT> &Hits,
                     const TargetInfoT &TargetInfo);
};
