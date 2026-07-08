#pragma once


//one window to start initial search with params (max depth, positive negative offset,).

#include "types.h"

class PointerW {
  bool initW();
  void endW();

  bool is_on_search_window_ = true;

  uint64_t tmp_target_adr_;

  int32_t tmp_scan_before_ = ptr_search_before;
  int32_t tmp_scan_after_ = ptr_search_after;
  uint8_t depth_limit_ = ptr_depth_limit;


  PendingAction cycleSearchW();

  void cyclePointerListW(const std::vector<PointerChain>& chains);


public:
    bool enabled_ = true;
    PendingAction cycleW(const std::vector<PointerChain>& chains, ScanType scan_type);


};
