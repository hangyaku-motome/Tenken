#pragma once

#include <format>
#include <string>
#include <vector>

namespace Log {

namespace Sink {
void writeLog(std::string msg, bool is_error);
};

std::vector<std::string> getLogText();

template <typename... Args> void info(std::format_string<Args...> format, Args&&... args) {
  Sink::writeLog(std::format(format, std::forward<Args>(args)...), false);
}

template <typename... Args> void error(std::format_string<Args...> format, Args&&... args) {
  Sink::writeLog(std::format(format, std::forward<Args>(args)...), true);
}

void openStream();
}  // namespace Log
