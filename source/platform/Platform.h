#pragma once

#include <filesystem>

namespace Platform {
bool checkPermission();
std::filesystem::path getImGuiInitPath();
std::filesystem::path getSavePath();
  
}
