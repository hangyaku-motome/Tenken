#pragma once

#include <cstdint>
#include <string>

struct WindowInfo {
  float H = 0;
  float W = 0;
  int XPos = 0;
  int YPos = 0;
};

struct DisplayInfo {
  int display_w = 0;
  int display_h = 0;

  float TopMenuHeight = 0;

  WindowInfo Hit;
  WindowInfo Log;
  WindowInfo Search;
};

struct ProcessInfo {
  int pid = 0;
  std::string FieldComm = "";
  std::string FieldCmdline = "";
  int uid = 0;
};

struct LogEvents {
  uint64_t ProcCount = 0;
  ProcessInfo ChosenProc;
  uint64_t HitCount = 0;
};

// Should hold persistent information like chosen target...And IDK what else for
// now.
struct ActiveInfo {
  ProcessInfo Target;
};
