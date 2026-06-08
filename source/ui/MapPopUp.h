#pragma once

#include <vector>

#include "types.h"

class MapsPopUp {
  std::vector<MapInfoT> regions_;
  void InitPopUp();

public:
  bool clicked_ = false;
  bool refresh_;

  void UpdateRegions(std::vector<MapInfoT> regions);
  void CyclePUp();
};
