#include "SearchW.h"

#include <imgui.h>

#include <cstdint>
#include <vector>

#include "display.h"
#include "Log.h"
#include "types.h"
#include "utils.h"

bool SearchW::initW() { return ImGui::Begin("Search"); }

void SearchW::endW() { ImGui::End(); }

PendingAction SearchW::cycleFirstW(const TargetInfo& target_info) {
  initW();

  PendingAction return_action{};

  auto type = getTargetType(target_info.target_type);

  if (type != TargetType::invalid) {
    Log::info("Chosen target type: {}", targetTypeToStr(type));
    return_action = Action::SetTargetInfo{{}, {}, type};
  };

  if (target_info.target_type == TargetType::aob) {
    std::vector<uint8_t> bytes = target_info.value;
    std::vector<bool> mask;
    if (target_info.mask.has_value()) mask = target_info.mask.value();
    if (strToAOBInfo(bytes, mask)) {
      return_action = Action::SetTargetInfo{bytes, mask, target_info.target_type};
      is_init_value_given_ = true;
    }

    if (target_info.value.empty() && !is_unknown_value_scan_) is_init_value_given_ = false;

  } else if (getTargetValue(target_info.target_type, tmp_val_)) {
    return_action = Action::SetTargetInfo{tmp_val_, {}, target_info.target_type};
    is_init_value_given_ = true;
  }

  if (target_info.target_type != TargetType::invalid && target_info.target_type != TargetType::aob)
    if (ImGui::Checkbox("Unknown inital value.", &is_unknown_value_scan_)) {
      is_init_value_given_ = is_unknown_value_scan_;
      tmp_val_.clear();
    }

  if (is_unknown_value_scan_ && target_info.target_type == TargetType::string) {
    ImGui::Text("Unknown value scanning with type string\nis not supported.");
    ImGui::BeginDisabled();
  } else
    ImGui::BeginDisabled(!is_init_value_given_);

  bool pressed_scan = ImGui::Button("Start First Scan!");
  ImGui::EndDisabled();
  endW();

  if (pressed_scan) {
    is_on_first_window_ = false;
    if (is_unknown_value_scan_) {
      is_on_unknown_value_first_scan = true;
      Log::info("Will start unknown value scan.");
      return Action::Scan::StartUnknownValue{};
    }
    dispatchType(target_info.target_type, [&]<typename T> { Log::info("Target value: {}", dataToStr<T>(tmp_val_)); });
    Log::info("Will start normal value scan.");
    return Action::Scan::StartNormal{.target_info = target_info};
  }
  return return_action;
}

PendingAction SearchW::cycleSecondW(const TargetInfo& TargetInfo) {
  if (!initW()) {
    endW();
    return {};
  }

  if (!is_on_unknown_value_first_scan)
    ImGui::Combo("Keep", &tmp_filter_type_, "unchanged\0changed\0increased\0decreased\0specific value\0\0");
  else
    ImGui::Combo("Keep", &tmp_filter_type_, "unchanged\0changed\0increased\0decreased\0\0");

  if (tmp_filter_type_ == 4) getTargetValue(TargetInfo.target_type, tmp_buf_);

  ImGui::BeginDisabled(tmp_filter_type_ == -1 || (tmp_buf_.empty() && tmp_filter_type_ == 4));
  if (ImGui::Button("Rescan!")) {
    is_on_unknown_value_first_scan = false;
    ImGui::EndDisabled();
    endW();
    if (tmp_filter_type_ == 4)
      return Action::FilterByValue{tmp_buf_};
    else {
      return Action::filterByStatus(static_cast<RelativeStatus>(tmp_filter_type_ + 1));
    }
  }
  ImGui::EndDisabled();
  float button_h = ImGui::GetFrameHeight();
  float button_w = 120.0F;
  float current_h = ImGui::GetContentRegionAvail().y;

  if (current_h > button_h) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h);
  }

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetContentRegionAvail().x - button_w) / 2));

  if (ImGui::Button("Undo scan.")) {
    endW();
    return Action::Scan::Undo{};
  }
  ImGui::SameLine();
  if (ImGui::Button("Restart scan.", {button_w, 0})) {
    endW();
    reset();
    return Action::Scan::Restart{};
  }

  endW();
  return {};
}

PendingAction SearchW::cycleW(TargetInfo& TargetInfo, const int32_t procID) {
  if (!procID) {
    initW();
    ImGui::Text("No process chosen yet.");
    endW();
    return {};
  }

  if (proc_id_ != procID) {
    reset();
    proc_id_ = procID;
  }

  if (is_on_first_window_) return cycleFirstW(TargetInfo);

  return cycleSecondW(TargetInfo);
}

void SearchW::reset() {
  is_init_value_given_ = false;
  is_unsigned_ = false;
  is_based_on_curr_val_ = false;
  is_unknown_value_scan_ = false;
  is_on_first_window_ = true;

  tmp_filter_type_ = -1;
  tmp_buf_.clear();
  tmp_val_.clear();
  tmp_target_type = -1;
}
