#include "HitsW.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

void HitsW::InitW(WindowInfo HitsWindow, ImGuiWindowFlags Flags) {
  ImGui::SetNextWindowPos(ImVec2(HitsWindow.XPos, HitsWindow.YPos));
  ImGui::SetNextWindowSize(ImVec2(HitsWindow.W, HitsWindow.H));

  ImGui::Begin("Hits", nullptr, Flags);
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
}
void HitsW::EndW() { ImGui::End(); }

void HitsW::CycleW(WindowInfo HitsWindow, ImGuiWindowFlags Flags) {
  InitW(HitsWindow, Flags);
  EndW();
}
