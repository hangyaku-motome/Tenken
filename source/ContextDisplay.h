#pragma once

#include "LogW.h"
#include "types.h"

template <typename T> struct ActionFor;

template <> struct ActionFor<HitInfoT> {
  using type = HitWAction;
};
template <> struct ActionFor<FavouriteInfoT> {
  using type = FavouriteWAction;
};
template <typename T> using ActionFor_t = typename ActionFor<T>::type;

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
      Log::Error("Entry " + std::to_string(Entry.location) +
                 " is near a memory region and I have not implemented a way to "
                 "reliably display bytes for that case. TODO later.");
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
  typename ActionFor<T>::type
  CycleContext(const uint64_t selected_row, const T Entry,
               const float RefreshDuration = 0,
               std::optional<bool> SetIsRefresh = std::nullopt) {
    if (SetIsRefresh.has_value())
      IsRefresh = SetIsRefresh.value();

    ActionFor_t<T> ReturnAction;

    DrawContextMenu(Entry);

    AlignButtons();

    auto RefreshContext = DrawRefreshContextButton();

    ImGui::SameLine();
    float NewRefreshDuration = DrawRefreshInterval(RefreshDuration);
    ImGui::SameLine();
    auto RefreshAll = DrawRefreshAllButton();

    if (RefreshContext)
      return ActionFor_t<T>{.Type = OpType::REFRESH, .index = selected_row};
    if (RefreshAll)
      return ActionFor_t<T>{.Type = OpType::REFRESH};

    if (NewRefreshDuration == -2)
      return {};

    ReturnAction.seconds = NewRefreshDuration;
    ReturnAction.Type = OpType::REGULAR_REFRESH;
    ReturnAction.index = selected_row;
    return ReturnAction;
  }
};
