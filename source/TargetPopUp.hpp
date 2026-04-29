#pragma once

#include "types.h"
#include <vector>

class TargetPopUp {
private:
  std::vector<ProcessInfoT> Processes;
  void Clicked(LogEventsT &LogEvents);

public:
  bool IsClicked = 0;

  void CyclePUp(LogEventsT &LogEvents, ActiveInfoT &ActiveInfo);
};