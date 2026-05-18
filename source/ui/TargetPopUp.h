#pragma once

#include "types.h"
#include <vector>

class TargetPopUp {
private:
  std::vector<ProcessInfoT> processes_;
  void InitPopUp();
  std::string search_;

public:
  bool clicked_ = false;

  PendingAction CyclePUp();
};