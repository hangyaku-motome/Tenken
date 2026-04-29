#pragma once

#include "imgui.h"
#include "types.h"
#include <string>

class LogW {

private:
  std::string LogText;
  void InitW(WindowInfo LogsWindow, ImGuiWindowFlags flagsWindowDefault);
  void EndW();
  void UpdateLog(LogEvents Events);

public:
  void CycleW(WindowInfo LogsWindow, ImGuiWindowFlags flagsWindowDefault,
              LogEvents Events);
};
