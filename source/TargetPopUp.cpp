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

OpType TargetPopUp::CyclePUp(ProcessInfoT &TargetProc) {
  if (IsClicked)
    Clicked();

  OpType return_val = OpType::NONE;

  if (ImGui::BeginPopupModal("Target List", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_AlwaysVerticalScrollbar |
                                 ImGuiWindowFlags_HorizontalScrollbar)) {
    ImGui::TextUnformatted("List targets here:");

    if (ImGui::Button("Refresh")) {
      Processes = ActOS::GetProcTargets();

      Log::Info("Found PID count: " + std::to_string(Processes.size()) + "\n");
    }

    if (ImGui::BeginTable("Targets", 4)) {
      for (const auto &Target : Processes) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(std::to_string(Target.pid).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(Target.FieldComm.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(Target.FieldCmdline.c_str());
        ImGui::TableNextColumn();

        if (ImGui::Selectable(std::to_string(ImGui::TableGetRowIndex()).c_str(),
                              false, ImGuiSelectableFlags_SpanAllColumns)) {
          TargetProc = Target;
          return_val = OpType::INIT_SCANNER;
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
  return return_val;
}