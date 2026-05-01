#pragma once

#include "imgui.h"
#include "types.h"
#include <string>

class LogW {

private:
  void InitW(WindowInfoT LogsWindow, ImGuiWindowFlags flagsWindowDefault);
  void EndW();

public:
  void CycleW(WindowInfoT LogsWindow, ImGuiWindowFlags flagsWindowDefault);
};

namespace Log {
const std::vector<std::string> &GetLogsText();

void Info(std::string WrittenString);
void Error(std::string WrittenString);
} // namespace Log