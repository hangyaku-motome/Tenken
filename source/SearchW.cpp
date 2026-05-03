#include "SearchW.h"
#include "LogW.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <string>

void SearchW::InitW(WindowInfoT SearchWindow, ImGuiWindowFlags Flags) {
  ImGui::SetNextWindowPos(ImVec2(SearchWindow.XPos, SearchWindow.YPos));
  ImGui::SetNextWindowSize(ImVec2(SearchWindow.W, SearchWindow.H));
  ImGui::Begin("Search", nullptr, Flags);
}

void SearchW::EndW() { ImGui::End(); }

// For combo add a way so that TargetType and the output string buffer are
// directly connected?
void SearchW::GetTargetType(TargetTypeT &TargetType) {

  int ChosenType = static_cast<int>(TargetType);
  if (ImGui::Combo("Type", &ChosenType,
                   "int8\0int16\0int32\0int64\0float\0double\0string\0\0")) {
    TargetType = static_cast<enum TargetTypeT>(ChosenType);
    Log::Info("Chosen target type:" + TargetTypeToString(TargetType) + "\n");
  }
}

void SearchW::GetIsUnsigned(bool &IsUnsigned, bool IsInt) {
  ImGui::BeginDisabled(!IsInt);
  if (!IsInt)
    IsUnsigned = false;

  bool temp_IsUnsigned = IsUnsigned;
  ImGui::Checkbox("Unsigned", &IsUnsigned);
  if (temp_IsUnsigned != IsUnsigned) {
    switch (static_cast<int>(IsUnsigned)) {
    case 0:
      Log::Info("Will search as signed.\n");
      break;
    case 1:
      Log::Info("Will search as unsigned.\n");
    }
  }

  ImGui::EndDisabled();
}

void SearchW::GetTargetValue(TargetInfoT &TargetInfo) {
  ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
  ImGuiDataType DataType;

  switch (static_cast<int>(TargetInfo.IsUnsigned)) {
  case 1:
    switch (static_cast<int>(TargetInfo.TargetType)) {
    case 0:
      TargetInfo.value.resize(1);
      DataType = ImGuiDataType_U8;
      break;
    case 1:
      TargetInfo.value.resize(2);
      DataType = ImGuiDataType_U16;
      break;
    case 2:
      TargetInfo.value.resize(4);
      DataType = ImGuiDataType_U32;
      break;
    case 3:
      TargetInfo.value.resize(8);
      DataType = ImGuiDataType_U64;
      break;
    }
    break;
  case 0:
    switch (static_cast<int>(TargetInfo.TargetType)) {
    case 0:
      TargetInfo.value.resize(1);
      DataType = ImGuiDataType_S8;
      break;
    case 1:
      TargetInfo.value.resize(2);
      DataType = ImGuiDataType_S16;
      break;
    case 2:
      TargetInfo.value.resize(4);
      DataType = ImGuiDataType_S32;
      break;
    case 3:
      TargetInfo.value.resize(8);
      DataType = ImGuiDataType_S64;
      break;
    case 4:
      TargetInfo.value.resize(4);
      DataType = ImGuiDataType_Float;
      break;
    case 5:
      TargetInfo.value.resize(8);
      DataType = ImGuiDataType_Double;
      break;
    case 6:
      // we will resize to null terminated bytes when scan begins.
      // ...still didn't do that by the way ^
      TargetInfo.value.resize(256, 0);
      if (ImGui::InputText("Value",
                           reinterpret_cast<char *>(TargetInfo.value.data()),
                           TargetInfo.value.size())) {
        InitValueGiven = true;
      };
      return;
    case 7:
      return;
    }
  }

  if (ImGui::InputScalar("Value", DataType, TargetInfo.value.data())) {
    InitValueGiven = true;
  };
}

int SearchW::CycleW(WindowInfoT SearchWindow, ImGuiWindowFlags Flags,
                    ChosenParams &ActiveInfo) {
  InitW(SearchWindow, Flags);
  if (!ActiveInfo.TargetProc.pid) {
    ImGui::Text("No target chosen yet.");
    ImGui::End();
    return -1;
  }
  if (IsOnFirstScanWindow) {

    GetTargetType(ActiveInfo.TargetValInfo.TargetType);

    bool IsInt = (static_cast<int>(ActiveInfo.TargetValInfo.TargetType) <= 3);
    GetIsUnsigned(ActiveInfo.TargetValInfo.IsUnsigned, IsInt);

    // We should check if it also empty, not just "Init value given" which
    // changes on any change. "" is not valid.
    GetTargetValue(ActiveInfo.TargetValInfo);
    ImGui::BeginDisabled(!InitValueGiven);
    bool PressedScan = ImGui::Button("Start First Scan!");
    ImGui::EndDisabled();
    if (PressedScan) {
      IsOnFirstScanWindow = false;
      EndW();
      return 1;
    }
    EndW();
    return 0;
  }
  EndW();
  return 0;
}

void SearchW::ClearWindow() {
  bool InitValueGiven = false;
  bool IsOnFirstScanWindow = true;
}