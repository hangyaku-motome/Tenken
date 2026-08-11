#pragma once

#include <cstdint>
#include <variant>

#include "types.h"

namespace ContextIntent {
struct Rescan {
  uint64_t index;
};

struct RescanAll {};

struct Write {
  uint64_t index;
};

struct RegularRefresh {
  float seconds;
};
}  // namespace ContextIntent

using ContextResult = std::variant<std::monostate,
                                   ContextIntent::Rescan,
                                   ContextIntent::RescanAll,
                                   ContextIntent::Write,
                                   ContextIntent::RegularRefresh>;

class ContextDisplay {
  float button_w_ = 150.0F;
  float button_h_ = 150.0F;
  float slider_w_ = 150.0F;
  float checkbox_w_ = 50.0F;
  bool is_refresh_ = false;

  float drawRefreshInterval(float RefreshDuration);
  bool drawRefreshAllButton() const;
  bool drawRefreshContextButton() const;
  void alignButtons();

  template <typename T> static void drawContextMenu(const T entry) {
    if (entry.value.size() + BytesBefore + BytesAfter != entry.bytes_around.size()) {
      return;
    }

    for (uint64_t i = 0; i < entry.bytes_around.size(); ++i) {
      ImGui::SameLine(0, 4);
      if (i % 32 == 0)
        ImGui::NewLine();
      else if (i % 8 == 0) {
        ImGui::Text(" ");
        ImGui::SameLine(0, 4);
      }
      if (i >= BytesBefore && i + BytesAfter < entry.bytes_around.size()) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 160, 100, 255));
        ImGui::Text("%02X", entry.bytes_around[i]);
        ImGui::PopStyleColor();
      } else
        ImGui::Text("%02X", entry.bytes_around[i]);
    }
  }

public:
  template <typename T> ContextResult cycleContext(const uint64_t selected_row, const T entry, float refresh_seconds) {
    if (refresh_seconds >= 0)
      is_refresh_ = true;
    else
      is_refresh_ = false;

    ContextResult ReturnAction{};

    drawContextMenu(entry);

    alignButtons();

    auto RefreshContext = drawRefreshContextButton();

    ImGui::SameLine();
    float NewRefreshDuration = drawRefreshInterval(refresh_seconds);
    if (NewRefreshDuration != -2) {
      ReturnAction = ContextIntent::RegularRefresh(NewRefreshDuration);
    }
    ImGui::SameLine();
    auto RefreshAll = drawRefreshAllButton();

    if (RefreshContext) return ContextIntent::Rescan{selected_row};
    if (RefreshAll) return ContextIntent::RescanAll{};

    return ReturnAction;
  }

  PendingAction ResolveContextIntent(ContextResult& cont, bool IsHitWindow) {
    PendingAction result{};
    std::visit(overloaded{
                   [&](ContextIntent::Rescan& c) -> void {
                     if (IsHitWindow)
                       result = Action::Hit::Rescan{c.index};
                     else
                       result = Action::Favourite::Rescan{c.index};
                   },
                   [&](ContextIntent::RescanAll&) -> void {
                     if (IsHitWindow)
                       result = Action::Hit::RescanAll{};
                     else
                       result = Action::Favourite::RescanAll{};
                   },
                   [&](ContextIntent::RegularRefresh& c) -> void {
                     if (IsHitWindow)
                       result = Action::Hit::RegularRefresh{c.seconds};
                     else
                       result = Action::Favourite::RegularRefresh{c.seconds};
                   },
                   [&](auto&) {},
               },
               cont);
    return result;
  }
};
