#include <unistd.h>

#include <fstream>
#include <iostream>

#include "Platform.h"

bool Platform::checkPermission() {
  if (geteuid() == 0) return true;

  std::ifstream permission("/proc/self/status");
  std::string line;
  uint64_t perms;

  while (std::getline(permission, line)) {
    if (!line.starts_with("CapEff")) continue;
    perms = std::strtoull(line.data() + line.find_first_of(':') + 2, nullptr, 16);
    if ((perms >> 19) & 1) return true;
  }

  std::ifstream permission_ptrace("/proc/sys/kernel/yama/ptrace_scope");
  int32_t perms_ptrace;
  permission_ptrace >> perms_ptrace;
  if (perms_ptrace == 0) return true;

  return false;
}

std::filesystem::path Platform::getTenkenStatePath() {
  char* user = getenv("SUDO_USER") ? getenv("SUDO_USER") : getenv("USER");
  std::filesystem::path path = std::filesystem::path("/home") / user / ".local" / "state" / "Tenken";
  std::filesystem::create_directories(path);
  return path;
}

std::filesystem::path Platform::getImguiInitPath() { return getTenkenStatePath() / "imgui.ini"; }

std::filesystem::path getTenkenSharePath() {
  char* user = getenv("SUDO_USER") ? getenv("SUDO_USER") : getenv("USER");
  std::filesystem::path path = std::filesystem::path("/home") / user / ".local" / "share" / "Tenken";
  std::filesystem::create_directories(path);
  return path;
}
