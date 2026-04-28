#include "ScanProc.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "start_end.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>

struct LogInfo {
  int pidCount = 0;
  bool TargetChosen = 0;
};

int main() {

  GLFWwindow *window = initalise_main();

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  ImGuiWindowFlags flagsHits = ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoCollapse;

  bool IsTargetMenuClicked = false;

  std::vector<ProcessInfo> Processes;
  LogInfo LogParams;
  std::string LogText;
  ProcessInfo ChosenTarget;

  // Main loop.
  while (!glfwWindowShouldClose(window)) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    float half_display_w = display_w * 0.5f;
    float half_display_h = display_h * 0.5f;
    float HitWindowWidth = half_display_w * 1.3;
    float HitWindowHeight = half_display_h * 1.1;

    start_frame();

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Target")) {
          IsTargetMenuClicked = true;
        }
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    float menu_height = ImGui::GetFrameHeight();

    if (IsTargetMenuClicked) {
      ImGui::OpenPopup("Target List");
      Processes = ProcessScanner();
      LogParams.pidCount = Processes.size();
      IsTargetMenuClicked = false;
    }

    // Target List.
    if (ImGui::BeginPopupModal("Target List", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_AlwaysVerticalScrollbar |
                                   ImGuiWindowFlags_HorizontalScrollbar)) {
      ImGui::Text("List targets here:");

      if (ImGui::Button("Refresh")) {
        Processes = ProcessScanner();
        LogParams.pidCount = Processes.size();
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

          if (ImGui::Selectable(
                  std::to_string(ImGui::TableGetRowIndex()).c_str(), false,
                  ImGuiSelectableFlags_SpanAllColumns)) {
            ChosenTarget = Target;
            LogParams.TargetChosen = true;
          }
        }
        ImGui::EndTable();
      }

      if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    // Hits.
    {
      ImGui::SetNextWindowPos(ImVec2(0, menu_height));
      ImGui::SetNextWindowSize(ImVec2(HitWindowWidth, HitWindowHeight));

      ImGui::Begin("Hits", nullptr, flagsHits);

      if (ImGui::BeginTable("table2", 3)) {
        // Use ImGuiClipper at some point.
        for (int row = 0; row < 40000; row++) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%d", row);
          ImGui::TableNextColumn();
          ImGui::Text("0x%d", 0x100 + row * 1000);
          ImGui::TableNextColumn();
          ImGui::Text("%d", 500 + row * 5);
        }
        ImGui::EndTable();
      }

      ImGui::End();
    }

    // Search.
    {
      ImGui::SetNextWindowPos(ImVec2(0 + HitWindowWidth, menu_height));
      ImGui::SetNextWindowSize(
          ImVec2(display_w - HitWindowWidth, HitWindowHeight));
      ImGui::Begin("Search", nullptr, flagsHits);

      ImGui::End();
    }

    // Log.
    {
      if (LogParams.pidCount) {
        std::stringstream tempss;
        tempss << "...Found PID count: " << LogParams.pidCount << "\n";
        LogParams.pidCount = 0;
        LogText += tempss.str();
      }
      if (LogParams.TargetChosen) {
        std::stringstream tempss;
        LogParams.TargetChosen = false;
        tempss << "...Chosen PID: " << ChosenTarget.pid
               << "   Target Comm:" << ChosenTarget.FieldComm
               << "   Target CmdLine:" << ChosenTarget.FieldCmdline << "\n";
        LogText += tempss.str();
        std::cout << LogText;
      }
      ImGui::SetNextWindowPos(ImVec2(0, menu_height + HitWindowHeight));
      ImGui::SetNextWindowSize(
          ImVec2(display_w, display_h - menu_height - HitWindowHeight));
      ImGui::Begin("Log", nullptr, flagsHits);
      ImGui::TextUnformatted(LogText.c_str(), LogText.end().base());

      ImGui::End();
    }

    end_frame(display_w, display_h, clear_color, window);
  }

  // Clean up.
  exit_main(window);
}