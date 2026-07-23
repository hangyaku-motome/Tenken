#pragma once

#include <filesystem>

namespace Platform {
bool checkPermission();

std::filesystem::path getTenkenStatePath();
std::filesystem::path getImguiInitPath();
std::filesystem::path getTenkenSharePath();
}  // namespace Platform
