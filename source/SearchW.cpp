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
void SearchW::GetTargetType(TargetTypeT &TargetType) {

  if (ImGui::Combo("Type", &TempTargetType,
                   "int8\0int16\0int32\0int64\0float\0double\0string\0\0")) {
    TargetType = static_cast<TargetTypeT>(TempTargetType + 4);
    Log::Info("Chosen target type:" + TargetTypeToString(TargetType) + "\n");
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
  }

  ImGui::EndDisabled();
}

inline std::string SearchW::TargetTypeToString(TargetTypeT TargetType) {
  switch (TargetType) {
  case TargetTypeT::Int8:
    return "Int8";
  case TargetTypeT::Int16:
    return "Int16";
  case TargetTypeT::Int32:
    return "Int32";
  case TargetTypeT::Int64:
    return "Int64";
  case TargetTypeT::Float:
    return "Float";
  case TargetTypeT::Double:
    return "Double";
  case TargetTypeT::String:
    return "String";
  default:
    return "Invalid";
  }
}

OpType SearchW::CycleFirstW(TargetInfoT &TargetInfo, bool TargetProcChosen) {
  InitW();
  if (TargetProcChosen == false) {
    ImGui::Text("No target chosen yet.");
    ImGui::End();
    return OpType::NONE;
  }

  GetTargetType(TargetInfo.TargetType);

  // We should check if it also empty, not just "Init value given" which
  // changes on any change. "" is not valid.
  if (GetTargetValue(TargetInfo.TargetType, TargetInfo.value) == true)
    InitValueGiven = true;
  ImGui::BeginDisabled(!InitValueGiven);
  bool PressedScan = ImGui::Button("Start First Scan!");
  ImGui::EndDisabled();
  EndW();
  if (PressedScan) {
    IsOnFirstScanWindow = false;
    return OpType::FIRST_SCAN;
  } else
    return OpType::NONE;

  return OpType::NONE;
}

Action SearchW::CycleSecondW(TargetInfoT &TargetInfo) {
  InitW();

  if (ImGui::Combo(
          "Keep", &TempFilterType,
          "unchanged\0changed\0increased\0decreased\0specific value\0\0")) {
  }
  if (TempFilterType == 4) {
    GetTargetValue(TargetInfo.TargetType, TargetInfo.value);
  }

  if (TempFilterType != -1) {
    // would not make sense to include this in first rescan...Well, at least
    // it must be after hits are rescaned once.

    ImGui::Checkbox("Based on CURRENT values?", &BasedOnCurrentValues);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(
          "When ticked, it will not fetch the latest data, instead it will "
          "filter based on current tags. Useful for when you rescan from "
          "hits "
          "window and want to work with that snapshot of values.");
    }
  }

  if (ImGui::Button("Rescan!")) {
    Action ReturnVal;
    ReturnVal.Type = OpType::FILTER;
    ReturnVal.BasedOnCurrentValues = BasedOnCurrentValues;
    ReturnVal.WorkOn = DataType::HIT;
    if (TempFilterType != 4)
      ReturnVal.KeepType = static_cast<RelativeStatus>(TempFilterType);
    EndW();
    return ReturnVal;
  }
  EndW();
  return Action{OpType::NONE};
}