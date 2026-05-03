#include "TargetPopUp.hpp"
#include "LogW.h"
#include "imgui.h"
#include "platform/ActOS.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <string>
void TargetPopUp::Clicked() {
  Processes = ActOS::GetProcTargets();
  Log::Info("Found PID count: " + std::to_string(Processes.size()) + "\n");
  ImGui::OpenPopup("Target List");
  IsClicked = false;
}

std::string TargetPopUp::CyclePUp(ChosenParams &ChosenParams) {
  if (IsClicked)
    Clicked();

  int return_val = 0;

  if (ImGui::BeginPopupModal("Target List", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_AlwaysVerticalScrollbar |
                                 ImGuiWindowFlags_HorizontalScrollbar)) {
    ImGui::Text("List targets here:");

    if (ImGui::Button("Refresh")) {
      Processes = ActOS::GetProcTargets();

      Log::Info("Found PID count: " + std::to_string(Processes.size()) + "\n");
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
          ChosenParams.TargetProc = Target;
          return_val = 1;
          Log::Info("...Chosen PID: " + std::to_string(Target.pid) +
                    "   Target Comm:" + Target.FieldComm +
                    "   Target CmdLine:" + Target.FieldCmdline + "\n");
        }
      }
      ImGui::EndTable();
    }

    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  if (return_val == 1)
    return "new target";
  else
    return "";
}
