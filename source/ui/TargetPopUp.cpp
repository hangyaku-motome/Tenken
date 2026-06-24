#include "TargetPopUp.h"

#include <string>

#include "imgui.h"
#include "LogW.h"
#include "misc/cpp/imgui_stdlib.cpp"
#include "platform/ActOS.h"
#include "types.h"


void TargetPopUp::InitPopUp() {
  processes_ = ActOS::GetProcTargets();
  Log::Info("Found PID count: " + std::to_string(processes_.size()) + "\n");
  ImGui::OpenPopup("Target List");
  clicked_ = false;
}

PendingAction TargetPopUp::CyclePUp() {
  if (clicked_) InitPopUp();

  PendingAction ReturnAction{};

  if (!ImGui::BeginPopupModal("Target List", nullptr, popup_flags)) return {};

  ImGui::TextUnformatted("List targets here:");

  ImGui::InputText("Filter name:", &search_);

  if (!ImGui::BeginTable("Targets", 3)) return {};

  for (const auto& Target : processes_) {
    if (!search_.empty() && Target.name.find(search_) == std::string::npos) continue;
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(std::to_string(Target.pid).c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(Target.name.c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(Target.cmdline.c_str());
    ImGui::PushID(ImGui::TableGetRowIndex());
    ImGui::SameLine();
    if (ImGui::Selectable("##selectable", false, ImGuiSelectableFlags_SpanAllColumns)) {
      ReturnAction = Action::TargetProcChosen{Target};
      Log::Info("...Chosen PID: " + std::to_string(Target.pid) + "   Target name:" + Target.name +
                "   Target cmdline:" + Target.cmdline + "\n");
    }
    ImGui::PopID();
  }
  ImGui::EndTable();

  if (ImGui::Button("Cancel")) {
    ImGui::CloseCurrentPopup();
  }

  if (ImGui::Button("Refresh")) {
    processes_ = ActOS::GetProcTargets();

    Log::Info("Found PID count: " + std::to_string(processes_.size()) + "\n");
  }

  ImGui::EndPopup();
  return ReturnAction;
}
