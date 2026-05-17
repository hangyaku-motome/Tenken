#pragma once

#include "types.h"
#include <cstdint>
#include <variant>

namespace ContextIntent {
struct rescan {
  uint64_t index;
};
struct rescanAll {};
struct write {
  uint64_t index;
};
struct regularRefresh {
  float seconds;
};
} // namespace ContextIntent

using ContextResult =
    std::variant<std::monostate, ContextIntent::rescan,
                 ContextIntent::rescanAll, ContextIntent::write,
                 ContextIntent::regularRefresh>;

class ContextDisplay {
  float button_w = 150.0F;
  float button_h = 150.0F;
  float slider_w = 150.0F;
  float checkbox_w = 50.0F;
  bool IsRefresh = false;

  float DrawRefreshInterval(float RefreshDuration);
  bool DrawRefreshAllButton() const;
  bool DrawRefreshContextButton() const;
  void AlignButtons();

  template <typename T> static void DrawContextMenu(const T Entry) {
    if (Entry.bytes_around.size() !=
        Entry.value.size() + BYTES_BEFORE + BYTES_AFTER) {
      return;
    }

    for (uint64_t i = 0; i < Entry.bytes_around.size(); ++i) {
      ImGui::SameLine(0, 4);
      if (i % 32 == 0)
        ImGui::NewLine();
      else if (i % 8 == 0) {
        ImGui::Text(" ");
        ImGui::SameLine(0, 4);
      }
      if (i >= BYTES_BEFORE && i + BYTES_AFTER < Entry.bytes_around.size()) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 160, 100, 255));
        ImGui::Text("%02X", Entry.bytes_around[i]);
        ImGui::PopStyleColor();
      } else
        ImGui::Text("%02X", Entry.bytes_around[i]);
    }
  }

public:
  template <typename T>
  ContextResult CycleContext(const uint64_t selected_row, const T Entry,
                             float refreshSeconds) {
    if (refreshSeconds >= 0)
      IsRefresh = true;
    else
      IsRefresh = false;

    ContextResult ReturnAction{};

    DrawContextMenu(Entry);

    AlignButtons();

    auto RefreshContext = DrawRefreshContextButton();

    ImGui::SameLine();
    float NewRefreshDuration = DrawRefreshInterval(refreshSeconds);
    if (NewRefreshDuration != -2) {
      ReturnAction = ContextIntent::regularRefresh(NewRefreshDuration);
    }
    ImGui::SameLine();
    auto RefreshAll = DrawRefreshAllButton();

    if (RefreshContext)
      return ContextIntent::rescan{selected_row};
    if (RefreshAll)
      return ContextIntent::rescanAll{};

    return ReturnAction;
  }

  PendingAction ResolveContextIntent(ContextResult &cont, bool IsHitWindow) {
    PendingAction result{};
    std::visit(overloaded{
                   [&](ContextIntent::rescan &c) -> void {
                     if (IsHitWindow)
                       result = Action::rescanHit{c.index};
                     else
                       result = Action::rescanFavourite{c.index};
                   },
                   [&](ContextIntent::rescanAll &) -> void {
                     if (IsHitWindow)
                       result = Action::rescanAllHits{};
                     else
                       result = Action::rescanAllFavourites{};
                   },
                   [&](ContextIntent::regularRefresh &c) -> void {
                     if (IsHitWindow)
                       result = Action::regularRefreshHits{c.seconds};
                     else
                       result = Action::regularRefreshFavourite{c.seconds};
                   },
                   [&](auto &) {},
               },
               cont);
    return result;
  }
};
