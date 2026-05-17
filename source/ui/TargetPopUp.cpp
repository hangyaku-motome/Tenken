#include "TargetPopUp.h"
#include "LogW.h"
#include "imgui.h"
#include "platform/ActOS.h"
#include "types.h"
#include <string>

void TargetPopUp::Clicked() {
  Processes = ActOS::GetProcTargets();
  Log::Info("Found PID count: " + std::to_string(Processes.size()) + "\n");
  ImGui::OpenPopup("Target List");
  IsClicked = false;
}

PendingAction TargetPopUp::CyclePUp() {
  if (IsClicked)
    Clicked();

  PendingAction ReturnAction{};

  if (!ImGui::BeginPopupModal("Target List", nullptr,
                              ImGuiWindowFlags_AlwaysAutoResize |
                                  ImGuiWindowFlags_AlwaysVerticalScrollbar |
                                  ImGuiWindowFlags_HorizontalScrollbar))
    return {};

  ImGui::TextUnformatted("List targets here:");

  if (!ImGui::BeginTable("Targets", 4))
    return {};

  for (const auto &Target : Processes) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(std::to_string(Target.pid).c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(Target.name.c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(Target.cmdline.c_str());
    ImGui::TableNextColumn();

    if (ImGui::Selectable(std::to_string(ImGui::TableGetRowIndex()).c_str(),
                          false, ImGuiSelectableFlags_SpanAllColumns)) {
      ReturnAction = Action::TargetProcChosen{Target};
      Log::Info("...Chosen PID: " + std::to_string(Target.pid) +
                "   Target Comm:" + Target.name +
                "   Target CmdLine:" + Target.cmdline + "\n");
    }
  }
  ImGui::EndTable();

  if (ImGui::Button("Cancel")) {
    ImGui::CloseCurrentPopup();
  }

  if (ImGui::Button("Refresh")) {
    Processes = ActOS::GetProcTargets();

    Log::Info("Found PID count: " + std::to_string(Processes.size()) + "\n");
  }

  ImGui::EndPopup();
  return ReturnAction;
}
