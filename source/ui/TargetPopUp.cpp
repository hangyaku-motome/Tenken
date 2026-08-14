#include "TargetPopUp.h"

#include <string>

#include "imgui.h"
#include "Log.h"
#include "misc/cpp/imgui_stdlib.cpp"
#include "platform/ProcessOS.h"
#include "types.h"

void TargetPopUp::initPopUp() {
  processes_ = ProcessOS::getProcTargets();
  Log::info("Found process count: {}", processes_.size());
  ImGui::OpenPopup("Target List");
  clicked_ = false;
}

PendingAction TargetPopUp::cyclePopUp() {
  if (clicked_) initPopUp();

  if (!ImGui::BeginPopupModal("Target List", nullptr, DefaultPopupFlags)) return {};

  PendingAction return_action{};

  ImGui::TextUnformatted("List targets here:");

  ImGui::InputText("Filter name:", &search_);

  if (!ImGui::BeginTable("Targets", 3)) return {};

  for (const auto& target : processes_) {
    if (!search_.empty() && target.name.find(search_) == std::string::npos) continue;
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(std::to_string(target.pid).c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(target.name.c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(target.cmdline.c_str());
    ImGui::PushID(ImGui::TableGetRowIndex());
    ImGui::SameLine();
    if (ImGui::Selectable("##selectable", false, ImGuiSelectableFlags_SpanAllColumns)) {
      return_action = Action::TargetProcChosen{target};
      Log::info("...Chosen PID: {}, Target name:{}, Target cmdline:{}", target.pid, target.name, target.cmdline);
    }
    ImGui::PopID();
  }
  ImGui::EndTable();

  if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();

  if (ImGui::Button("Refresh")) {
    processes_ = ProcessOS::getProcTargets();
    Log::info("Found process count: {}", processes_.size());
  }

  ImGui::EndPopup();
  return return_action;
}
