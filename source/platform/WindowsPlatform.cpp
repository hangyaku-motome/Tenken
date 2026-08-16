#include <windows.h>

#include <filesystem>

#include "Platform.h"

// I hate windows I hate windoww I ahte whiwndo iahtei wodiwndIthwwindwisIw hate hTREAHTEHATEHATEWW

bool Platform::checkPermission() {
  HANDLE token;
  TOKEN_ELEVATION elevation;
  OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token);
  DWORD returned_size;
  GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &returned_size);
  CloseHandle(token);

  return elevation.TokenIsElevated != 0;
}

std::filesystem::path Platform::getImguiInitPath() {
  const wchar_t* localappdata = _wgetenv(L"LOCALAPPDATA");
  if (!localappdata) localappdata = L".";
  return getTenkenStatePath() / L"imgui.ini";
}

std::filesystem::path Platform::getTenkenStatePath() {
  const wchar_t* local_app_data = _wgetenv(L"LOCALAPPDATA");
  if (!local_app_data) local_app_data = L".";
  return std::filesystem::path(local_app_data) / L"Tenken";
}

std::filesystem::path Platform::getTenkenSharePath() {
  const wchar_t* local_app_data = _wgetenv(L"LOCALAPPDATA");
  if (!local_app_data) local_app_data = L".";
  return std::filesystem::path(local_app_data) / L"Tenken";
}
