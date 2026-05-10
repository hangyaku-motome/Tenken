#include "LogW.h"
#include "imgui.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <mutex>
#include <string>
#include <vector>

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
std::mutex LogMutex;
std::vector<std::string> Logs;
} // namespace

const std::vector<std::string> GetLogsText() {
  std::scoped_lock<std::mutex> lock(LogMutex);
  return Logs;
}

void Info(std::string WrittenString) {
  std::scoped_lock<std::mutex> lock(LogMutex);
  Logs.push_back(WrittenString);
}
void Error(std::string WrittenString) {
  std::scoped_lock<std::mutex> lock(LogMutex);
  Logs.push_back("ERROR: " + WrittenString);
}

} // namespace Log
