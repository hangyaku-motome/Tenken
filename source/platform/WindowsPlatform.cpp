#include "Platform.h"
#include <windows.h>

// not implemented yet. Need to check if admin.
bool Platform::checkPermission() {
  return 1;
}

std::filesystem::path Platform::getImGuiInitPath() {
  return std::filesystem::path(getenv("LOCALAPPDATA")) / "Tenken" / "imgui.ini";
}

std::filesystem::path Platform::getSavePath() {
  return std::filesystem::path(getenv("APPDATA")) / "Tenken" / "tenkenSave.json";
}
  
