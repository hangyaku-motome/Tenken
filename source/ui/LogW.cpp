#include "LogW.h"

#include <mutex>
#include <string>
#include <vector>

#include "imgui.h"

bool LogW::initW() { return ImGui::Begin("Log"); }

void LogW::endW() { ImGui::End(); }

void LogW::cycleW() {
  if (!enabled_) return;

  if (!initW()) {
    endW();
    return;
  }
  for (const auto& text : Log::getLogText()) {
    ImGui::TextUnformatted(text.c_str());
  }
  endW();
}

namespace Log {
namespace {
std::mutex log_mutex;
std::vector<std::string> logs;
}  // namespace

std::vector<std::string> getLogText() {
  std::scoped_lock<std::mutex> lock(log_mutex);
  return logs;
}

void info(const std::string& written_string) {
  std::scoped_lock<std::mutex> lock(log_mutex);
  logs.push_back(written_string);
}

void error(const std::string& written_string) {
  std::scoped_lock<std::mutex> lock(log_mutex);
  logs.push_back("ERROR: " + written_string);
}

}  // namespace Log
