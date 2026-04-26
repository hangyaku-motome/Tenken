#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "start_end.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdio.h>

int main() {

  GLFWwindow *window = initalise_main();

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  ImGuiWindowFlags flagsHits = ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoCollapse;

  bool SEARCH_TARGET = 0;

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
          SEARCH_TARGET = 1;
        }
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    float menu_height = ImGui::GetFrameHeight();

    // Hits.
    {
      ImGui::SetNextWindowPos(ImVec2(0, menu_height));
      ImGui::SetNextWindowSize(ImVec2(HitWindowWidth, HitWindowHeight));

      ImGui::Begin("Hits", nullptr, flagsHits);

      if (ImGui::BeginTable("table2", 3)) {
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
      ImGui::SetNextWindowPos(ImVec2(0, menu_height + HitWindowHeight));
      ImGui::SetNextWindowSize(
          ImVec2(display_w, display_h - menu_height - HitWindowHeight));
      ImGui::Begin("Log", nullptr, flagsHits);

      ImGui::End();
    }

    end_frame(display_w, display_h, clear_color, window);
  }

  // Clean up.
  exit_main(window);
}