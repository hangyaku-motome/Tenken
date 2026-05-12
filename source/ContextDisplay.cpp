#include "ContextDisplay.h"

void ContextDisplay::AlignButtons() {
  button_h = ImGui::GetFrameHeight();
  button_w = 150.0F;
  float current_h = ImGui::GetContentRegionAvail().y;

  if (current_h > button_h) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h);
  }

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ((ImGui::GetContentRegionAvail().x - button_w) / 2));
}

bool ContextDisplay::DrawRefreshContextButton() const {
  return ImGui::Button("Refresh Context Entry", {button_w, 0});
}

bool ContextDisplay::DrawRefreshAllButton() const {
  float current_h = ImGui::GetContentRegionAvail().y;
  if (current_h > button_h) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h);
  }
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ImGui::GetContentRegionAvail().x - button_w);

  return ImGui::Button("Refresh All Entries", {button_w, 0});
}

float ContextDisplay::DrawRefreshInterval(const float RefreshDuration) {
  float DisplaySeconds = RefreshDuration < 0.3 ? 0 : RefreshDuration;
  float returnval = -2;

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ImGui::GetContentRegionAvail().x - slider_w -
                       checkbox_w - 25);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                       ImGui::GetContentRegionAvail().y - 50);

  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Will regularly refresh entry each given duration.\n");
  }
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ImGui::GetContentRegionAvail().x - slider_w -
                       checkbox_w);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                       ImGui::GetContentRegionAvail().y - 50);
  if (ImGui::Checkbox("##Regular Refresh", &IsRefresh)) {
    if (!IsRefresh) {
      returnval = -1;
    }
    if (IsRefresh)
      returnval = 0;
  }
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ImGui::GetContentRegionAvail().x - slider_w);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                       ImGui::GetContentRegionAvail().y - 50);

  ImGui::SetNextItemWidth(slider_w);

  if (!IsRefresh)
    ImGui::BeginDisabled();
  if (ImGui::SliderFloat("##interval", &DisplaySeconds, 0.3F, 3.0F, "%.1f",
                         ImGuiSliderFlags_AlwaysClamp)) {
    return DisplaySeconds;
  }

  if (!IsRefresh)
    ImGui::EndDisabled();

  return returnval;
}
