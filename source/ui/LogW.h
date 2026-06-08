#pragma once

#include <string>
#include <vector>

class LogW {
private:
  static bool InitW();
  static void EndW();

public:
  bool enabled_ = false;
  void CycleW();
};

namespace Log {
std::vector<std::string> GetLogsText();

void Info(const std::string& WrittenString);
void Error(const std::string& WrittenString);
}  // namespace Log
