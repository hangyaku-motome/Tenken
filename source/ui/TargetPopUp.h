#pragma once

#include <vector>

#include "types.h"

class TargetPopUp {
private:
  std::vector<ProcessInfoT> processes_;
  void initPopUp();
  std::string search_;

public:
  bool clicked_ = false;

  PendingAction cyclePopUp();
};
