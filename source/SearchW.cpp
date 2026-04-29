#include "SearchW.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

void SearchW::InitW(WindowInfo SearchWindow, ImGuiWindowFlags Flags) {
  ImGui::SetNextWindowPos(ImVec2(SearchWindow.XPos, SearchWindow.YPos));
  ImGui::SetNextWindowSize(ImVec2(SearchWindow.W, SearchWindow.H));
  ImGui::Begin("Search", nullptr, Flags);
}

void SearchW::EndW() { ImGui::End(); }

void SearchW::CycleW(WindowInfo SearchWindow, ImGuiWindowFlags Flags) {
  InitW(SearchWindow, Flags);
  EndW();
}
