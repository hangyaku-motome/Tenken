#pragma once

#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class SearchW {
private:
  bool InitValueGiven = false;
  void InitW(WindowInfoT SearchWindow, ImGuiWindowFlags Flags);
  void EndW();
  void GetTargetType(TargetTypeT &TargetType);
  void GetIsUnsigned(bool &IsUnsigned, bool IsInt);
  void GetTargetValue(TargetInfoT &TargetInfo);

  bool IsOnFirstScanWindow = true;

public:
  WindowInfoT Window;

  int CycleW(ChosenParams &ActiveInfo);

  void ClearWindow();
};
