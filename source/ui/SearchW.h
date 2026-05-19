#pragma once

#include "types.h"
#include <cstdint>
#include <imgui.h>

class SearchW {
private:
  static bool InitW();
  static void EndW();
  bool GetTargetType(TargetTypeT &writeTo);
  std::string GetHitFilter(TargetInfoT &TargetInfo);

  // these variables can be moved inside, at least some.
  bool InitValueGiven = false;
  bool IsUnsigned = false;

  bool BasedOnCurrentValues = false;

  bool UnknownValueScan = false;

  SessionState::SearchWStatus old_status;

  int32_t TempFilterType = -1;
  std::vector<uint8_t> tempbuf;

  std::vector<uint8_t> tempval;

  int TempTargetType = -1;

public:
  PendingAction CycleFirstW(const TargetInfoT &TargetInfo);
  PendingAction CycleSecondW(const TargetInfoT &TargetInfo, bool IsUnknownnValueScan);

  PendingAction CycleW(TargetInfoT &TargetInfo, SessionState::SearchWStatus State,
                       bool IsUnknownValueScan) {
    if (State != old_status) {
      if (State == SessionState::SearchWStatus::FIRST) {

        InitValueGiven = false;
        IsUnsigned = false;
        BasedOnCurrentValues = false;
        UnknownValueScan = false;
        TempFilterType = -1;
        TempTargetType = -1;
        tempbuf.clear();
        tempval.clear();
      }
    }
    old_status = State;
    switch (State) {
    case SessionState::SearchWStatus::DISABLED:
      InitW();
      ImGui::Text("No target chosen.");
      EndW();
      return {};
    case SessionState::SearchWStatus::FIRST:
      return CycleFirstW(TargetInfo);
    case SessionState::SearchWStatus::SECOND:
      return CycleSecondW(TargetInfo, IsUnknownValueScan);
    }
    return {};
  }
};
