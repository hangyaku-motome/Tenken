#pragma once

#include <string>
#include <vector>

class LogW {

private:
  static void InitW();
  static void EndW();

public:
  static void CycleW();
};

namespace Log {
std::vector<std::string> GetLogsText();

void Info(const std::string &WrittenString);
void Error(const std::string &WrittenString);
} // namespace Log