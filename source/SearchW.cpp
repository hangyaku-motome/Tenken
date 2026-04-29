#include "SearchW.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

void SearchW::InitW(WindowInfoT SearchWindow, ImGuiWindowFlags Flags) {
  ImGui::SetNextWindowPos(ImVec2(SearchWindow.XPos, SearchWindow.YPos));
  ImGui::SetNextWindowSize(ImVec2(SearchWindow.W, SearchWindow.H));
  ImGui::Begin("Search", nullptr, Flags);
}

void SearchW::EndW() { ImGui::End(); }

void SearchW::CycleW(WindowInfoT SearchWindow, ImGuiWindowFlags Flags) {
  InitW(SearchWindow, Flags);
  EndW();
}
