#pragma once

#include <imgui-filebrowser/imfilebrowser.h>

#include <filesystem>

#include "Platform.h"
#include "PointerList.h"
#include "types.h"

class PointerW {
  bool initW();
  void endW();

  static constexpr auto file_browser_flags = ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CloseOnEsc |
                                             ImGuiFileBrowserFlags_CreateNewDir | ImGuiFileBrowserFlags_EditPathString;

  Pointer::InitConfig init_config;

  int64_t prev_display_start_ = -1;
  int64_t prev_display_end_ = -1;

  ImGui::FileBrowser file_browser_{file_browser_flags};

  PendingAction cycleSearchW();

  void cyclePointerListW(PointerList& pointer_list);

  std::vector<Pointer::PrettyChain> chains_;

public:
  PointerW() {
    std::filesystem::create_directory(Platform::getTenkenStatePath() / "Pointer");
    // TODO: at the start of program, create all directories that might be used.
    file_browser_.SetDirectory(Platform::getTenkenStatePath() / "Pointer");
  };

  bool enabled_ = true;
  PendingAction cycleW(const SessionState& state, PointerList& pointer_list);
};
