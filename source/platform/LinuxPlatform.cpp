#include "Platform.h"

#include <unistd.h>

#include <fstream>

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

std::filesystem::path Platform::getImGuiInitPath() {
  char* user = getenv("SUDO_USER") ? getenv("SUDO_USER") : getenv("USER");
  return std::filesystem::path(user) / ".local" / "state" / "Tenken" / "imgui.ini";
}

std::filesystem::path Platform::getSavePath() {
  char* user = getenv("SUDO_USER") ? getenv("SUDO_USER") : getenv("USER");
  return std::filesystem::path(user) / ".local" / "state" / "Tenken" / "tenkenSave.json";
}
