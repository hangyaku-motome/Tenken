#pragma once

#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class HitsW {
private:
  void InitW(WindowInfoT HitsWindow, ImGuiWindowFlags Flags);
  void EndW();
  void DrawHitTable(const std::vector<HitInfoT> &Hits,
                    const TargetInfoT &TargetInfo);
  void DrawContextMenu(const HitInfoT Hit);

  int64_t selected_row = 0;

public:
  void CycleW(WindowInfoT HitsWindow, ImGuiWindowFlags Flags,
              const std::vector<HitInfoT> &Hits, const TargetInfoT &TargetInfo);
};
