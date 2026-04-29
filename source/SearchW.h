#pragma once

#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class SearchW {
private:
  void InitW(WindowInfoT SearchWindow, ImGuiWindowFlags Flags);
  void EndW();

public:
  void CycleW(WindowInfoT SearchWindow, ImGuiWindowFlags Flags);
};
