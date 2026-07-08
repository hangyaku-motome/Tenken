#pragma once

#include <unordered_set>
#include <vector>

#include "Scanner.h"
#include "types.h"

class MapsPopUp {
  std::vector<MapInfo> regions_;
  std::unordered_set<uint64_t> active_addresses_;
  bool refresh_;
  bool first_launch_ = true;

  struct checkboxes {
    bool code = false;
    bool read_only_const = false;
    bool heap = true;
    bool anon = true;
    bool main_exec_data = true;
    bool lib_data = false;
  };

  checkboxes filter_;

  void initPopUp();
  void updateRegions();
  void renderTable();
  void applyDefaultFilters();
  void toggleFilter(MapType type, bool enable);
  std::vector<MapInfo> buildFilteredMap();

  const Scanner* scanner_;

public:
  explicit MapsPopUp(const Scanner& scanner)
      : scanner_(&scanner) {}

  bool clicked_ = false;
  void cyclePopUp(std::vector<MapInfo>& ActiveRegions);
};
