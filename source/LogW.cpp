#include "LogW.h"
#include "imgui.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

std::string LogText;

void LogW::InitW() { ImGui::Begin("Log"); }

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
void Error(std::string WrittenString) {
  Logs.push_back("ERROR: " + WrittenString);
}

} // namespace Log
