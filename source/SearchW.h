#pragma once

#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class SearchW {
private:
  static void InitW();
  static void EndW();
  bool GetTargetType(TargetTypeT &TargetType);
  std::string GetHitFilter(TargetInfoT &TargetInfo);

  bool InitValueGiven = false;
  int TempTargetType = -1;
  bool IsUnsigned = false;
  int TempFilterType = -1;

  bool IsOnFirstScanWindow = true;
  bool BasedOnCurrentValues = false;

public:
  OpType CycleFirstW(TargetInfoT &TargetInfo, bool TargetProcChosen);
  SearchWAction CycleSecondW(TargetInfoT &TargetInfo);

  SearchWAction CycleW(TargetInfoT &TargetInfo, bool TargetProcChosen) {
    if (IsOnFirstScanWindow)
      return SearchWAction{.Type = CycleFirstW(TargetInfo, TargetProcChosen),
                           .KeepType = RelativeStatus::UNSET};

    return CycleSecondW(TargetInfo);
  }
};
