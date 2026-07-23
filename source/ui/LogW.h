#pragma once

#include <string>
#include <vector>

// TODO: make it print log save location on start up.
class LogW {
private:
  static bool initW();
  static void endW();

public:
  bool enabled_ = false;
  void cycleW();
};

namespace Log {
std::vector<std::string> getLogText();

void info(const std::string& written_string);
void error(const std::string& written_string);
bool openStream();
}  // namespace Log
