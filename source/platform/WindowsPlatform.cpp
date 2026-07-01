#include "Platform.h"
#include <windows.h>

// not implemented yet. Need to check if admin.
bool Platform::checkPermission() {
  HANDLE token;
  TOKEN_ELEVATION elevation;
  OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token);
  DWORD returned_size;
  GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &returned_size);
  CloseHandle(token);

  return elevation.TokenIsElevated != 0;
}

std::filesystem::path Platform::getImGuiInitPath() {
  return std::filesystem::path(getenv("LOCALAPPDATA")) / "Tenken" / "imgui.ini";
}

std::filesystem::path Platform::getSavePath() {
  return std::filesystem::path(getenv("APPDATA")) / "Tenken" / "tenkenSave.json";
}
  
