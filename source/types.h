#pragma once

#include <cstdint>
#include <string>

struct WindowInfoT {
  float H = 0;
  float W = 0;
  int XPos = 0;
  int YPos = 0;
};

struct DisplayInfoT {
  int display_w = 0;
  int display_h = 0;

  float TopMenuHeight = 0;

  WindowInfoT Hit;
  WindowInfoT Log;
  WindowInfoT Search;
};

struct ProcessInfoT {
  int pid = 0;
  std::string FieldComm = "";
  std::string FieldCmdline = "";
  int uid = 0;
};

struct LogEventsT {
  uint64_t ProcCount = 0;
  ProcessInfoT ChosenProc;
  uint64_t HitCount = 0;
};

// Should hold persistent information like chosen target...And IDK what else for
// now.
struct ActiveInfoT {
  ProcessInfoT Target;
};
