#pragma once

#include <imgui.h>

#include <cstdint>

#include "types.h"

class SearchW {
  static bool initW();
  static void endW();

  bool getTargetType(TargetType& write_to);
  std::string getHitFilter(TargetInfoT& target_info);

  bool is_init_value_given_ = false;
  bool is_unsigned_ = false;
  bool is_based_on_curr_val_ = false;
  bool is_unknown_value_scan_ = false;
  bool is_on_first_window_ = true;

  int32_t tmp_filter_type_ = -1;
  std::vector<uint8_t> tmp_buf_;
  std::vector<uint8_t> tmp_val_;
  int tmp_target_type = -1;

  int proc_id_ = 0;

  PendingAction cycleFirstW(const TargetInfoT& target_info);
  PendingAction cycleSecondW(const TargetInfoT& target_info);

  void reset();

public:
  PendingAction cycleW(TargetInfoT& target_info, int32_t proc_id);
};
