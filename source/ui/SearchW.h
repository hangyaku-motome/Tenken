#pragma once

#include <imgui.h>

#include <cstdint>

#include "types.h"

class SearchW {
  static bool InitW();
  static void EndW();

  bool GetTargetType(TargetTypeT& writeTo);
  std::string GetHitFilter(TargetInfoT& TargetInfo);

  bool isInitValueGiven_ = false;
  bool IsUnsigned_ = false;
  bool isBasedOnCurrentValues_ = false;
  bool isUnknownValueScan_ = false;
  bool isOnFirstWindow_ = true;

  int32_t tmpFilterType_ = -1;
  std::vector<uint8_t> tmpBuf_;
  std::vector<uint8_t> tmpVal_;
  int tmpTargetType_ = -1;

  int procID_ = 0;

  PendingAction CycleFirstW(const TargetInfoT& TargetInfo);
  PendingAction CycleSecondW(const TargetInfoT& TargetInfo);

  void reset();

public:
  PendingAction CycleW(TargetInfoT& TargetInfo, int32_t procID);
};
