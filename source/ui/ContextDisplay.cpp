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
