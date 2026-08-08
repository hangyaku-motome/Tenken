#pragma once

#include <imgui-filebrowser/imfilebrowser.h>
#include <imgui.h>

#include <cstdint>
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

  Window windows;

  uint64_t adr_buf = 0;

  ImGui::FileBrowser save_dialog;
  ImGui::FileBrowser load_dialog;
  ImGui::FileBrowser ptr_dialog;

public:
  MainMenuBar(bool& target_popup, bool& map_popup, bool& log_w, bool& hex_w, bool& data_inspector, bool& pointer_w)
      : windows{.target_popup = target_popup,
                .map_popup = map_popup,
                .log_w = log_w,
                .hex_w = hex_w,
                .data_inspector = data_inspector,
                .pointer_w = pointer_w} {}

  PendingAction cycle(const std::string& exec_name);
};
