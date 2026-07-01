#include "Platform.h"
#include <windows.h>
#include <filesystem>

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
  const wchar_t* localappdata = _wgetenv(L"LOCALAPPDATA");
  if(!localappdata) localappdata = L".";
  return std::filesystem::path(localappdata) / L"Tenken" / L"imgui.ini";
}

std::filesystem::path Platform::getSavePath() {
  const wchar_t* appdata = _wgetenv(L"APPDATA");
  if(!appdata) appdata = L".";
  return std::filesystem::path(appdata) / L"Tenken" / L"tenkenSave.json";
}
