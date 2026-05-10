#pragma once

#include <string>
#include <vector>

class LogW {

private:
  void InitW();
  void EndW();

public:
  void CycleW();
};

namespace Log {
const std::vector<std::string> GetLogsText();

void Info(std::string WrittenString);
void Error(std::string WrittenString);
} // namespace Log