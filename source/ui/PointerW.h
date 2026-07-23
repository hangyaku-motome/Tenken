#pragma once

// one window to start initial search with params (max depth, positive negative offset,).

#include <imgui-filebrowser/imfilebrowser.h>

#include "PointerList.h"
#include "types.h"

class PointerW {
  bool initW();
  void endW();

  bool is_on_search_window_ = true;

  Pointer::InitConfig init_config;

  int64_t prev_display_start_ = -1;
  int64_t prev_display_end_ = -1;

  bool is_all_chains_ = false;

  ImGui::FileBrowser file_browser_;

  PendingAction cycleSearchW();

  void cyclePointerListW(PointerList& pointer_list);

  std::vector<Pointer::PrettyChain> chains_;

public:
  bool enabled_ = true;
  PendingAction cycleW(const SessionState& state, PointerList& pointer_list);
};
