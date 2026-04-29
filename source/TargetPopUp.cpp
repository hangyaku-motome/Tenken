#include "TargetPopUp.hpp"
#include "imgui.h"
#include "platform/ActOS.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
void TargetPopUp::Clicked(LogEventsT &LogEvents) {
  Processes = ActOS::GetTargets();
  LogEvents.ProcCount = Processes.size();
  ImGui::OpenPopup("Target List");
  IsClicked = false;
}

void TargetPopUp::CyclePUp(LogEventsT &LogEvents, ActiveInfoT &ActiveInfo) {
  if (IsClicked)
    Clicked(LogEvents);

  if (ImGui::BeginPopupModal("Target List", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_AlwaysVerticalScrollbar |
                                 ImGuiWindowFlags_HorizontalScrollbar)) {
    ImGui::Text("List targets here:");

    if (ImGui::Button("Refresh")) {
      Processes = ActOS::GetTargets();
      LogEvents.ProcCount = Processes.size();
    }

    if (ImGui::BeginTable("Targets", 4)) {
      for (const auto &Target : Processes) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", std::to_string(Target.pid).c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%s", Target.FieldComm.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%s", Target.FieldCmdline.c_str());
        ImGui::TableNextColumn();

        if (ImGui::Selectable(std::to_string(ImGui::TableGetRowIndex()).c_str(),
                              false, ImGuiSelectableFlags_SpanAllColumns)) {
          ActiveInfo.Target = Target;
          LogEvents.ChosenProc = ActiveInfo.Target;
        }
      }
      ImGui::EndTable();
    }

    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}
