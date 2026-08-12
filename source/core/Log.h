#pragma once

#include <string>
#include <vector>

namespace Log {
std::vector<std::string> getLogText();

void info(const std::string& written_string);
void error(const std::string& written_string);
void openStream();
}  // namespace Log
