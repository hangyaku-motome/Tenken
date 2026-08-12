#include "ContextDisplay.h"

void ContextDisplay::alignButtons() {
  button_h_ = ImGui::GetFrameHeight();
  button_w_ = 150.0F;
  float current_h = ImGui::GetContentRegionAvail().y;

  if (current_h > button_h_) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h_);
  }

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetContentRegionAvail().x - button_w_) / 2));
}

bool ContextDisplay::drawRefreshContextButton() const { return ImGui::Button("Refresh Context Entry", {button_w_, 0}); }

bool ContextDisplay::drawRefreshAllButton() const {
  float current_h = ImGui::GetContentRegionAvail().y;
  if (current_h > button_h_) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h_);
  }
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - button_w_);

  return ImGui::Button("Refresh All Entries", {button_w_, 0});
}

float ContextDisplay::drawRefreshInterval(const float RefreshDuration) {
  float DisplaySeconds = RefreshDuration < 0.3 ? 0 : RefreshDuration;
  float returnval = -2;

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - slider_w_ - checkbox_w_ - 25);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 50);

  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Will regularly refresh entry each given duration.\n");
  }
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - slider_w_ - checkbox_w_);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 50);
  if (ImGui::Checkbox("##Regular Refresh", &is_refresh_)) {
    if (!is_refresh_) {
      returnval = -1;
    }
    if (is_refresh_) returnval = 0;
  }
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - slider_w_);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 50);

  ImGui::SetNextItemWidth(slider_w_);

  if (!is_refresh_) ImGui::BeginDisabled();
  if (ImGui::SliderFloat("##interval", &DisplaySeconds, 0.3F, 3.0F, "%.1f", ImGuiSliderFlags_AlwaysClamp)) {
    return DisplaySeconds;
  }

  if (!is_refresh_) ImGui::EndDisabled();

  return returnval;
}

template <typename T>
PendingAction ContextDisplay::cycleContext(const ReadBlock& context,
                                           uint64_t selected_row,
                                           uint32_t target_size,
                                           float refresh_seconds) {
  if (refresh_seconds >= 0)
    is_refresh_ = true;
  else
    is_refresh_ = false;

  PendingAction action{};

  drawContextMenu(context, target_size);

  alignButtons();

  auto refresh_context = drawRefreshContextButton();

  ImGui::SameLine();
  float new_refresh_duration = drawRefreshInterval(refresh_seconds);
  if (new_refresh_duration != -2) {
    if constexpr (std::is_same_v<T, HitInfo>)
      action = Action::Hit::RegularRefresh(new_refresh_duration);
    else
      action = Action::Favourite::RegularRefresh(new_refresh_duration);
  }
  ImGui::SameLine();
  auto refresh_all = drawRefreshAllButton();

  if (refresh_context) {
    if constexpr (std::is_same_v<T, HitInfo>)
      action = Action::Hit::Rescan(selected_row);
    else
      action = Action::Favourite::Rescan(selected_row);
  }

  if (refresh_all) {
    if constexpr (std::is_same_v<T, HitInfo>)
      action = Action::Hit::RescanAll{};
    else
      action = Action::Favourite::RescanAll{};
  }

  return action;
};

template PendingAction ContextDisplay::cycleContext<HitInfo>(const ReadBlock&, uint64_t, uint32_t, float);
template PendingAction ContextDisplay::cycleContext<FavouriteInfo>(const ReadBlock&, uint64_t, uint32_t, float);

void ContextDisplay::drawContextMenu(const ReadBlock& context, uint32_t target_size) {
  for (int64_t i = 0; i < context.read_bytes.size(); ++i) {
    ImGui::SameLine(0, 4);
    if (i % 32 == 0)
      ImGui::NewLine();
    else if (i % 8 == 0) {
      ImGui::Text(" ");
      ImGui::SameLine(0, 4);
    }
    if (i + context.offset_from_adr >= 0 && i + context.offset_from_adr < target_size) {
      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 160, 100, 255));
      ImGui::Text("%02X", context.read_bytes[i]);
      ImGui::PopStyleColor();
    } else
      ImGui::Text("%02X", context.read_bytes[i]);
  }
}
