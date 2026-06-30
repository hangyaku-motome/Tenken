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

  if (ImGui::Combo("Type", &TempTargetType, "int8\0int16\0int32\0int64\0float\0double\0string\0AOB search\0\0")) {
    newType = static_cast<TargetTypeT>(TempTargetType + 4);
    Log::Info("Chosen target type:" + targetTypeToStr(newType) + "\n");
    Changed = true;
  }

  bool IsInt = (static_cast<int>(newType) <= 7);
  ImGui::BeginDisabled(!IsInt);
  if (!IsInt) IsUnsigned = false;

  bool temp_IsUnsigned = IsUnsigned;
  ImGui::Checkbox("Unsigned", &IsUnsigned);
  if (temp_IsUnsigned != IsUnsigned) {
    if (IsUnsigned) {
      Log::Info("Will search as unsigned.\n");
      newType = static_cast<TargetTypeT>(static_cast<int>(newType) - 4);
    } else {
      Log::Info("Will search as signed.\n");
      newType = static_cast<TargetTypeT>(TempTargetType + 4);
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
      InitValueGiven = true;
    }

    if (TargetInfo.value.empty() && !UnknownValueScan) InitValueGiven = false;

  } else if (GetTargetValue(TargetInfo.TargetType, tempval)) {
    ReturnAction = Action::setTargetInfo{TargetInfo.TargetType, tempval};
    InitValueGiven = true;
  }

  if (TargetInfo.TargetType != TargetTypeT::Invalid && TargetInfo.TargetType != TargetTypeT::AOB)
    if (ImGui::Checkbox("Unknown inital value.", &UnknownValueScan)) {
      InitValueGiven = UnknownValueScan;
    }

  if (UnknownValueScan && tempType == TargetTypeT::String) {
    ImGui::Text("Unknown value scanning with type string\nis not supported.");
    ImGui::BeginDisabled();
  } else
    ImGui::BeginDisabled(!InitValueGiven);

  bool PressedScan = ImGui::Button("Start First Scan!");
  ImGui::EndDisabled();
  EndW();

  if (PressedScan) {
    if (UnknownValueScan) return Action::startUnknownValueScan{};
    return Action::firstScan{.targetInfo = TargetInfo};
  }
  return ReturnAction;
}

PendingAction SearchW::CycleSecondW(const TargetInfoT& TargetInfo, bool IsUnknownValueScan) {
  if (!InitW()) {
    EndW();
    return {};
  }

  if (!IsUnknownValueScan)
    ImGui::Combo("Keep", &TempFilterType, "unchanged\0changed\0increased\0decreased\0specific value\0\0");
  else
    ImGui::Combo("Keep", &TempFilterType, "unchanged\0changed\0increased\0decreased\0\0");

  if (TempFilterType == 4) GetTargetValue(TargetInfo.TargetType, tempbuf);

  ImGui::BeginDisabled(TempFilterType == -1 || (tempbuf.empty() && TempFilterType == 4));
  if (ImGui::Button("Rescan!")) {
    ImGui::EndDisabled();
    EndW();
    if (TempFilterType == 4)
      return Action::filterByValue{tempbuf};
    else {
      return Action::filterByStatus(static_cast<RelativeStatus>(TempFilterType));
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
    return Action::restartScan{};
  }

  EndW();
  return {};
}
