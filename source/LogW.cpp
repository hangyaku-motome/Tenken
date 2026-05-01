#include "LogW.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

std::string LogText;

void LogW::InitW(WindowInfoT LogsWindow, ImGuiWindowFlags Flags) {
  ImGui::SetNextWindowPos(ImVec2(LogsWindow.XPos, LogsWindow.YPos));
  ImGui::SetNextWindowSize(ImVec2(LogsWindow.W, LogsWindow.H));
  ImGui::Begin("Log", nullptr, Flags);
}

void LogW::EndW() { ImGui::End(); }

void LogW::CycleW(WindowInfoT LogsWindow, ImGuiWindowFlags Flags) {
  InitW(LogsWindow, Flags);
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
