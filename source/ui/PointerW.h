#pragma once

#include <imgui-filebrowser/imfilebrowser.h>

#include <filesystem>

#include "Platform.h"
#include "PointerList.h"
#include "types.h"

class PointerW {
  bool initW();
  void endW();

  static constexpr auto FileBrowserFlags = ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CloseOnEsc |
                                           ImGuiFileBrowserFlags_CreateNewDir | ImGuiFileBrowserFlags_EditPathString;

  Pointer::InitConfig init_config_{};

  ImGui::FileBrowser file_browser_{FileBrowserFlags};

  PendingAction cycleSearchW(const State& state);
  PendingAction cyclePointerListW(PointerList& pointer_list);

  std::vector<Pointer::PrettyChain> chains_;

  int64_t favourite_index = -1;

public:
  PointerW() {
    std::filesystem::create_directory(Platform::getTenkenStatePath() / "Pointer");
    // TODO: at the start of program, create all directories that might be used.
    file_browser_.SetDirectory(Platform::getTenkenStatePath() / "Pointer");
  };

  bool enabled_ = true;
  PendingAction cycleW(const State& state, PointerList& pointer_list);
};
