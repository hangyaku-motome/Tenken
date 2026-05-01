#include "HitsW.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

void HitsW::InitW(WindowInfoT HitsWindow, ImGuiWindowFlags Flags) {
  ImGui::SetNextWindowPos(ImVec2(HitsWindow.XPos, HitsWindow.YPos));
  ImGui::SetNextWindowSize(ImVec2(HitsWindow.W, HitsWindow.H));

  ImGui::Begin("Hits", nullptr, Flags);
}

void HitsW::EndW() { ImGui::End(); }

void HitsW::CycleW(WindowInfoT HitsWindow, ImGuiWindowFlags Flags,
                   const std::vector<HitInfoT> &Hits) {
  InitW(HitsWindow, Flags);
  RenderTable(Hits);
  EndW();
}

// Unfinished.
void HitsW::RenderTable(const std::vector<HitInfoT> &Hits) {
  if (ImGui::BeginTable("Hit Table", 4)) {
    for (uint32_t row = 0; row < Hits.size(); ++row) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
    }
  }
}
