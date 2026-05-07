#pragma once

#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class SearchW {
private:
  void InitW(WindowInfoT SearchWindow, ImGuiWindowFlags Flags);
  void EndW();
  void GetTargetType(TargetTypeT &TargetType);
  void GetTargetValue(TargetInfoT &TargetInfo);
  inline std::string TargetTypeToString(TargetTypeT TargetType);
  std::string GetHitFilter(TargetInfoT &TargetInfo);

  bool InitValueGiven = false;

  int TempTargetType = -1;
  bool IsUnsigned = false;

public:
  bool IsOnFirstScanWindow = true;
  int TempFilterType = -1;
  bool BasedOnCurrentValues = false;

  WindowInfoT Window;

  std::string CycleFirstW(TargetInfoT &TargetInfo, bool TargetProcChosen);
  std::string CycleSecondW(TargetInfoT &TargetInfo);
};
