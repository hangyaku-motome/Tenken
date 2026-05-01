#pragma once

#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class HitsW {
private:
  void InitW(WindowInfoT HitsWindow, ImGuiWindowFlags Flags);
  void EndW();
  void RenderTable(const std::vector<HitInfoT> &Hits);

public:
  void CycleW(WindowInfoT HitsWindow, ImGuiWindowFlags Flags,
              const std::vector<HitInfoT> &Hits);
};
