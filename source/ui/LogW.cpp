#include "LogW.h"

#include <imgui.h>

#include <string>

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
