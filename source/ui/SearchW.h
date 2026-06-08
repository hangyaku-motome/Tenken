#pragma once

#include <imgui.h>

#include <cstdint>

#include "types.h"

class SearchW {
private:
  static bool InitW();
  static void EndW();
  bool GetTargetType(TargetTypeT& writeTo);
  std::string GetHitFilter(TargetInfoT& TargetInfo);

  // these variables can be moved inside, at least some.
  bool InitValueGiven = false;
  bool IsUnsigned = false;

  bool BasedOnCurrentValues = false;

  bool UnknownValueScan = false;

  SessionState::SearchWStatusT old_status;

  int32_t TempFilterType = -1;
  std::vector<uint8_t> tempbuf;

  std::vector<uint8_t> tempval;

  int TempTargetType = -1;

public:
  PendingAction CycleFirstW(const TargetInfoT& TargetInfo);
  PendingAction CycleSecondW(const TargetInfoT& TargetInfo, bool IsUnknownnValueScan);

  PendingAction CycleW(TargetInfoT& TargetInfo, SessionState::SearchWStatusT State, bool IsUnknownValueScan) {
    if (State != old_status) {
      if (State == SessionState::SearchWStatusT::FIRST) {
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
      case SessionState::SearchWStatusT::DISABLED:
        InitW();
        ImGui::Text("No target chosen.");
        EndW();
        return {};
      case SessionState::SearchWStatusT::FIRST:
        return CycleFirstW(TargetInfo);
      case SessionState::SearchWStatusT::SECOND:
        return CycleSecondW(TargetInfo, IsUnknownValueScan);
    }
    return {};
  }
};
