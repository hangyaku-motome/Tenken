#pragma once

#include "types.h"
#include <string>

class LogW {

private:
  void InitW();
  void EndW();

public:
  void CycleW();
  WindowInfoT Window;
};

namespace Log {
const std::vector<std::string> &GetLogsText();

void Info(std::string WrittenString);
void Error(std::string WrittenString);
} // namespace Log