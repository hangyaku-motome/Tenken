#pragma once

#include <imgui-filebrowser/imfilebrowser.h>
#include <imgui.h>

#include <cstdint>

#include "Platform.h"
#include "types.h"

class MainMenuBar {
  struct Window {
    bool& target_popup;
    bool& map_popup;
    bool& log_w;
    bool& hex_w;
    bool& data_inspector;
    bool& pointer_w;
  };

  static constexpr auto file_browser_flags = ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CloseOnEsc |
                                             ImGuiFileBrowserFlags_CreateNewDir | ImGuiFileBrowserFlags_EditPathString;

  Window windows_;

  uint64_t adr_buf_ = 0;

  // do I really need 3?
  // TODO: maybeee for save and load, setDirectory to ~/Documents or eqv on Windows.
  ImGui::FileBrowser save_dialog_{file_browser_flags};
  ImGui::FileBrowser load_dialog_{file_browser_flags};
  ImGui::FileBrowser ptr_dialog_{file_browser_flags};

  bool ptr_resolve_popup = false;

  PendingAction cyclePointerPopup(const std::string& exec_name);

  void cycleFileMenu();

  void cycleUtilsMenu();

public:
  MainMenuBar(bool& target_popup, bool& map_popup, bool& log_w, bool& hex_w, bool& data_inspector, bool& pointer_w)
      : windows_{.target_popup = target_popup,
                 .map_popup = map_popup,
                 .log_w = log_w,
                 .hex_w = hex_w,
                 .data_inspector = data_inspector,
                 .pointer_w = pointer_w} {
    ptr_dialog_.SetDirectory(Platform::getTenkenStatePath() / "Pointer");
  }

  PendingAction cycle(const std::string& target_name);
};
