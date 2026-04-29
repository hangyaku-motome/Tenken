#pragma once

#include "imgui.h"
#include "types.h"
#include <string>

class LogW {

private:
  std::string LogText;
  void InitW(WindowInfoT LogsWindow, ImGuiWindowFlags flagsWindowDefault);
  void EndW();
  void UpdateLog(LogEventsT Events);

public:
  void CycleW(WindowInfoT LogsWindow, ImGuiWindowFlags flagsWindowDefault,
              LogEventsT Events);
};
