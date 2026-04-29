#pragma once

#include "types.h"
#include <vector>

class TargetPopUp {
private:
  std::vector<ProcessInfo> Processes;
  void Clicked(LogEvents &LogEvents);

public:
  bool IsClicked = 0;

  void CyclePUp(LogEvents &LogEvents, ActiveInfo &ActiveInfo);
};