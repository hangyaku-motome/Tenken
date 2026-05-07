#include "SearchW.h"
#include "LogW.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <string>

// technically does not have to recieve them as args anymore...? Just as in
// other objects. Eh...Will deal with this fact later.
void SearchW::InitW(WindowInfoT SearchWindow, ImGuiWindowFlags Flags) {
  ImGui::SetNextWindowPos(ImVec2(SearchWindow.XPos, SearchWindow.YPos));
  ImGui::SetNextWindowSize(ImVec2(SearchWindow.W, SearchWindow.H));
  ImGui::Begin("Search", nullptr, Flags);
}

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

void SearchW::GetTargetValue(TargetInfoT &TargetInfo) {
  ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
  ImGuiDataType DataType;

  switch (TargetInfo.TargetType) {
  case TargetTypeT::uInt8:
    TargetInfo.value.resize(1);
    DataType = ImGuiDataType_U8;
    break;
  case TargetTypeT::uInt16:
    TargetInfo.value.resize(2);
    DataType = ImGuiDataType_U16;
    break;
  case TargetTypeT::uInt32:
    TargetInfo.value.resize(4);
    DataType = ImGuiDataType_U32;
    break;
  case TargetTypeT::uInt64:
    TargetInfo.value.resize(8);
    DataType = ImGuiDataType_U64;
    break;

  case TargetTypeT::Int8:
    TargetInfo.value.resize(1);
    DataType = ImGuiDataType_S8;
    break;
  case TargetTypeT::Int16:
    TargetInfo.value.resize(2);
    DataType = ImGuiDataType_S16;
    break;
  case TargetTypeT::Int32:
    TargetInfo.value.resize(4);
    DataType = ImGuiDataType_S32;
    break;
  case TargetTypeT::Int64:
    TargetInfo.value.resize(8);
    DataType = ImGuiDataType_S64;
    break;
  case TargetTypeT::Float:
    TargetInfo.value.resize(4);
    DataType = ImGuiDataType_Float;
    break;
  case TargetTypeT::Double:
    TargetInfo.value.resize(8);
    DataType =
        ImGuiDataType_Double; // I wonder if there is a way to "link types" so
                              // that TargetTypeT::Double can also be just
                              // ImGuiDataType_Double in necessary contexts?
                              // Not really useful here, though.
    break;
  case TargetTypeT::String:
    TargetInfo.value.resize(64, 0);
    if (ImGui::InputText("Value",
                         reinterpret_cast<char *>(TargetInfo.value.data()),
                         TargetInfo.value.size())) {
      InitValueGiven = true;
    };
    return;
  case TargetTypeT::Invalid:
    return;
  }
  if (ImGui::InputScalar("Value", DataType, TargetInfo.value.data())) {
    InitValueGiven = true;
  };
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

std::string SearchW::CycleFirstW(TargetInfoT &TargetInfo,
                                 bool TargetProcChosen) {
  InitW(Window, Window.flags);
  if (TargetProcChosen == false) {
    ImGui::Text("No target chosen yet.");
    ImGui::End();
    return "";
  }

  GetTargetType(TargetInfo.TargetType);

  // We should check if it also empty, not just "Init value given" which
  // changes on any change. "" is not valid.
  GetTargetValue(TargetInfo);
  ImGui::BeginDisabled(!InitValueGiven);
  bool PressedScan = ImGui::Button("Start First Scan!");
  ImGui::EndDisabled();
  EndW();
  if (PressedScan) {
    IsOnFirstScanWindow = false;
    return "initial scan";
  } else
    return "";

  return "";
}

// on rescan, a option for "based on current values?" which does not rescan
// hits. It only filters based on what already exists. This is useful for when
// the "rescan" on the Hits window is used and they wish to keep it based on
// those, not on the now-latest data. I thought about adding an "all" tag to
// rescan...But eh? I will like search should have the logic of filtering only
// what we want. That doesn't do that. We can refresh and then we can either
// keep it based on current tags, or refresh. This does add some more
// flexiliblity. We'll see how useful it'll be in practice.

// A dropdown to choose: Higher, Lower, Unchanged, Changed, Specific Value.

// If specific value, then take new value.

// A tick box for "Based on CURRENT tags?" ideally with a small explanation
// that goes something like "When ticked, it will not fetch the latest data,
// instead it will filter based on current tags. Useful for when you rescan
// from hits window and want to work with that snapshot of values.

// I'll keep these comments above until next time.

std::string SearchW::CycleSecondW(TargetInfoT &TargetInfo) {
  InitW(Window, Window.flags);

  if (ImGui::Combo(
          "Keep", &TempFilterType,
          "unchanged\0changed\0increased\0decreased\0specific value\0\0")) {
  }
  if (TempFilterType == 4)
    GetTargetValue(TargetInfo);

  if (TempFilterType != -1) {
    // would not make sense to include this in first rescan...Well, at least it
    // must be after hits are rescaned once.

    ImGui::Checkbox("Based on CURRENT values?", &BasedOnCurrentValues);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(
          "When ticked, it will not fetch the latest data, instead it will "
          "filter based on current tags. Useful for when you rescan from hits "
          "window and want to work with that snapshot of values.");
    }
  }

  std::string returnval;
  if (ImGui::Button("Rescan!")) {
    returnval = "rescan";
  } else
    returnval = "";

  EndW();

  return returnval;
}