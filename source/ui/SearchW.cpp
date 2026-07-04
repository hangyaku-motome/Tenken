#include "SearchW.h"

#include <cstdint>
#include <string>
#include <vector>

#include "display.h"
#include "imgui.h"
#include "LogW.h"
#include "types.h"
#include "utils.h"

bool SearchW::InitW() { return ImGui::Begin("Search"); }

void SearchW::EndW() { ImGui::End(); }

bool SearchW::GetTargetType(TargetTypeT& newType) {
  bool Changed = false;

  if (ImGui::Combo("Type", &tmpTargetType_, "int8\0int16\0int32\0int64\0float\0double\0string\0AOB search\0\0")) {
    newType = static_cast<TargetTypeT>(tmpTargetType_ + 4);
    Log::Info("Chosen target type:" + targetTypeToStr(newType) + "\n");
    Changed = true;
  }

  bool IsInt = (static_cast<int>(newType) <= 7);
  ImGui::BeginDisabled(!IsInt);
  if (!IsInt) IsUnsigned_ = false;

  bool temp_IsUnsigned = IsUnsigned_;
  ImGui::Checkbox("Unsigned", &IsUnsigned_);
  if (temp_IsUnsigned != IsUnsigned_) {
    if (IsUnsigned_) {
      Log::Info("Will search as unsigned.\n");
      newType = static_cast<TargetTypeT>(static_cast<int>(newType) - 4);
    } else {
      Log::Info("Will search as signed.\n");
      newType = static_cast<TargetTypeT>(tmpTargetType_ + 4);
    }
    Changed = true;
  }

  ImGui::EndDisabled();

  return Changed;
}

PendingAction SearchW::CycleFirstW(const TargetInfoT& TargetInfo) {
  InitW();

  PendingAction ReturnAction{};

  TargetTypeT tempType = TargetInfo.TargetType;
  if (GetTargetType(tempType)) ReturnAction = Action::setTargetInfo{tempType, {}};

  if (TargetInfo.TargetType == TargetTypeT::AOB) {
    std::vector<uint8_t> bytes = TargetInfo.value;
    std::vector<bool> mask;
    if (TargetInfo.mask.has_value()) mask = TargetInfo.mask.value();
    if (strToAOBInfo(bytes, mask)) {
      ReturnAction = Action::setTargetInfo{TargetInfo.TargetType, bytes, mask};
      isInitValueGiven_ = true;
    }

    if (TargetInfo.value.empty() && !isUnknownValueScan_) isInitValueGiven_ = false;

  } else if (GetTargetValue(TargetInfo.TargetType, tmpVal_)) {
    ReturnAction = Action::setTargetInfo{TargetInfo.TargetType, tmpVal_};
    isInitValueGiven_ = true;
  }

  if (TargetInfo.TargetType != TargetTypeT::Invalid && TargetInfo.TargetType != TargetTypeT::AOB)
    if (ImGui::Checkbox("Unknown inital value.", &isUnknownValueScan_)) {
      isInitValueGiven_ = isUnknownValueScan_;
    }

  if (isUnknownValueScan_ && tempType == TargetTypeT::String) {
    ImGui::Text("Unknown value scanning with type string\nis not supported.");
    ImGui::BeginDisabled();
  } else
    ImGui::BeginDisabled(!isInitValueGiven_);

  bool PressedScan = ImGui::Button("Start First Scan!");
  ImGui::EndDisabled();
  EndW();

  if (PressedScan) {
    isOnFirstWindow_ = false;
    if (isUnknownValueScan_) return Action::startUnknownValueScan{};
    return Action::firstScan{.targetInfo = TargetInfo};
  }
  return ReturnAction;
}

PendingAction SearchW::CycleSecondW(const TargetInfoT& TargetInfo) {
  if (!InitW()) {
    EndW();
    return {};
  }

  if (!isUnknownValueScan_)
    ImGui::Combo("Keep", &tmpFilterType_, "unchanged\0changed\0increased\0decreased\0specific value\0\0");
  else
    ImGui::Combo("Keep", &tmpFilterType_, "unchanged\0changed\0increased\0decreased\0\0");

  if (tmpFilterType_ == 4) GetTargetValue(TargetInfo.TargetType, tmpBuf_);

  ImGui::BeginDisabled(tmpFilterType_ == -1 || (tmpBuf_.empty() && tmpFilterType_ == 4));
  if (ImGui::Button("Rescan!")) {
    ImGui::EndDisabled();
    EndW();
    if (tmpFilterType_ == 4)
      return Action::filterByValue{tmpBuf_};
    else {
      return Action::filterByStatus(static_cast<RelativeStatus>(tmpFilterType_));
    }
  }
  ImGui::EndDisabled();
  float button_h = ImGui::GetFrameHeight();
  float button_w = 120.0F;
  float current_h = ImGui::GetContentRegionAvail().y;

  if (current_h > button_h) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h);
  }

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetContentRegionAvail().x - button_w) / 2));

  if (ImGui::Button("Undo scan.")) {
    EndW();
    return Action::undoScan{};
  }
  ImGui::SameLine();
  if (ImGui::Button("Restart scan.", {button_w, 0})) {
    EndW();
    reset();
    return Action::restartScan{};
  }

  EndW();
  return {};
}

PendingAction SearchW::CycleW(TargetInfoT& TargetInfo, const int32_t procID) {
  if (!procID) {
    InitW();
    ImGui::Text("No process chosen yet.");
    EndW();
    return {};
  }

  if (procID_ != procID) {
    reset();
    procID_ = procID;
  }

  if (isOnFirstWindow_) return CycleFirstW(TargetInfo);

  return CycleSecondW(TargetInfo);
}

void SearchW::reset() {
  isInitValueGiven_ = false;
  IsUnsigned_ = false;
  isBasedOnCurrentValues_ = false;
  isUnknownValueScan_ = false;
  isOnFirstWindow_ = true;

  tmpFilterType_ = -1;
  tmpBuf_.clear();
  tmpVal_.clear();
  tmpTargetType_ = -1;
}
