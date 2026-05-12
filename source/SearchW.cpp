#include "SearchW.h"
#include "LogW.h"
#include "display.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <string>

// technically does not have to recieve them as args anymore...? Just as in
// other objects. Eh...Will deal with this fact later.
void SearchW::InitW() { ImGui::Begin("Search"); }

void SearchW::EndW() { ImGui::End(); }

// For combo add a way so that TargetType and the output string buffer are
// directly connected?
// I do not know what the comment above is supposed to mean.
// Okay I do now. The problem is we are arbitarily writing the options and then
// seperately wiring it back to the enums. It's a bit fragile, isn't it?
bool SearchW::GetTargetType(TargetTypeT &TargetType) {
  bool Changed = false;

  if (ImGui::Combo("Type", &TempTargetType,
                   "int8\0int16\0int32\0int64\0float\0double\0string\0\0")) {
    TargetType = static_cast<TargetTypeT>(TempTargetType + 4);
    Log::Info("Chosen target type:" + TargetTypeToStr(TargetType) + "\n");
    Changed = true;
  }

  bool IsInt = (static_cast<int>(TargetType) <= 7);
  ImGui::BeginDisabled(!IsInt);
  if (!IsInt)
    IsUnsigned = false;

  bool temp_IsUnsigned = IsUnsigned;
  ImGui::Checkbox("Unsigned", &IsUnsigned);
  if (temp_IsUnsigned != IsUnsigned) {
    if (IsUnsigned) {
      Log::Info("Will search as unsigned.\n");
      TargetType = static_cast<TargetTypeT>(static_cast<int>(TargetType) - 4);
    } else {
      Log::Info("Will search as signed.\n");
      TargetType = static_cast<TargetTypeT>(TempTargetType + 4);
    }
    Changed = true;
  }

  ImGui::EndDisabled();

  return Changed;
}

OpType SearchW::CycleFirstW(TargetInfoT &TargetInfo, bool TargetProcChosen) {
  InitW();
  if (!TargetProcChosen) {
    ImGui::TextUnformatted("No target chosen yet.");
    ImGui::End();
    return OpType::NONE;
  }

  if (GetTargetType(TargetInfo.TargetType))
    TargetInfo.value.clear();

  if (GetTargetValue(TargetInfo.TargetType, TargetInfo.value))
    InitValueGiven = true;

  if (TargetInfo.value.empty())
    InitValueGiven = false;

  ImGui::BeginDisabled(!InitValueGiven);
  bool PressedScan = ImGui::Button("Start First Scan!");
  ImGui::EndDisabled();
  EndW();
  if (PressedScan) {
    IsOnFirstScanWindow = false;
    TempFilterType = -1;
    return OpType::FIRST_SCAN;
  }
  return OpType::NONE;
}

SearchWAction SearchW::CycleSecondW(TargetInfoT &TargetInfo) {
  InitW();

  if (ImGui::Combo(
          "Keep", &TempFilterType,
          "unchanged\0changed\0increased\0decreased\0specific value\0\0")) {
  }
  if (TempFilterType == 4)
    GetTargetValue(TargetInfo.TargetType, TargetInfo.value);

  if (TempFilterType != -1) {
    // would not make sense to include this in first rescan...Well, at least
    // it must be after hits are rescaned once.
    ImGui::Checkbox("Based on CURRENT values?", &BasedOnCurrentValues);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(
          "When ticked, it will not fetch the latest data, instead it will "
          "filter based on current tags. \nUseful for when you rescan from "
          "hits "
          "window and want to work with that snapshot of values.");
    }
  }

  ImGui::BeginDisabled(TempFilterType == -1);
  if (ImGui::Button("Rescan!")) {
    SearchWAction ReturnVal;
    ReturnVal.Type = OpType::FILTER;
    ReturnVal.BasedOnCurrentValues = BasedOnCurrentValues;
    if (TempFilterType != 4)
      ReturnVal.KeepType = static_cast<RelativeStatus>(TempFilterType);
    ImGui::EndDisabled();
    EndW();
    return ReturnVal;
  }
  ImGui::EndDisabled();

  float button_h = ImGui::GetFrameHeight();
  float button_w = 120.0F;
  float current_h = ImGui::GetContentRegionAvail().y;

  if (current_h > button_h) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h);
  }

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ((ImGui::GetContentRegionAvail().x - button_w) / 2));

  if (ImGui::Button("Restart scan.", {button_w, 0})) {
    SearchWAction ReturnVal;
    ReturnVal.Type = OpType::RESTART_STATE;
    EndW();
    return ReturnVal;
  }

  EndW();
  return {};
}