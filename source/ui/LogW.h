#pragma once

#include "Log.h"

class LogW {
private:
  static bool initW();
  static void endW();

  void openStream();

public:
  LogW() { Log::openStream(); }

  bool enabled_ = true;
  void cycleW();
};
