#include "LogW.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

std::string LogText;

void LogW::InitW() {
  ImGui::SetNextWindowPos(ImVec2(Window.XPos, Window.YPos));
  ImGui::SetNextWindowSize(ImVec2(Window.W, Window.H));
  ImGui::Begin("Log", nullptr, Window.flags);
}

void LogW::EndW() { ImGui::End(); }

void LogW::CycleW() {
  InitW();
  for (const auto &Text : Log::GetLogsText()) {
    ImGui::Text("%s", Text.c_str());
  }
  EndW();
}

namespace Log {
namespace {
std::vector<std::string> Logs;
}

const std::vector<std::string> &GetLogsText() { return Logs; }

void Info(std::string WrittenString) { Logs.push_back(WrittenString); }
// also implement different ones for error and idk other stuff. Oh and make the
// type some sort of LogInfo instead of string (for Logs) for better formatting.
void Error(std::string WrittenString) {
  Logs.push_back("ERROR: " + WrittenString);
}

} // namespace Log
