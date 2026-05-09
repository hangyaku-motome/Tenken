#pragma once

#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class SearchW {
private:
  void InitW();
  void EndW();
  void GetTargetType(TargetTypeT &TargetType);
  inline std::string TargetTypeToString(TargetTypeT TargetType);
  std::string GetHitFilter(TargetInfoT &TargetInfo);

  bool InitValueGiven = false;
  int TempTargetType = -1;
  bool IsUnsigned = false;
  int TempFilterType = -1;

public:
  bool IsOnFirstScanWindow = true;
  bool BasedOnCurrentValues = false;

  OpType CycleFirstW(TargetInfoT &TargetInfo, bool TargetProcChosen);
  Action CycleSecondW(TargetInfoT &TargetInfo);

  Action CycleW(TargetInfoT &TargetInfo, bool TargetProcChosen) {
    if (IsOnFirstScanWindow)
      return Action{CycleFirstW(TargetInfo, TargetProcChosen)};
    else
      return CycleSecondW(TargetInfo);
  }
};
