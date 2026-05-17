#pragma once

#include "types.h"
#include <vector>

class TargetPopUp {
private:
  std::vector<ProcessInfoT> Processes;
  void Clicked();

public:
  bool IsClicked = false;

  PendingAction CyclePUp();
};