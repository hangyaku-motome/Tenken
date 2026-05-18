#pragma once

#include "types.h"
#include <vector>

class TargetPopUp {
private:
  std::vector<ProcessInfoT> Processes;
  void InitPopUp();

public:
  bool clicked_ = false;

  PendingAction CyclePUp();
};