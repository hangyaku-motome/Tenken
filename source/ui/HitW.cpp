#include "HitW.h"

#include <imgui.h>

#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "ContextDisplay.h"
#include "display.h"
#include "types.h"
#include "utils.h"

bool HitW::initW() { return ImGui::Begin("Hits"); }

void HitW::endW() { ImGui::End(); }

PendingAction HitW::cycleW(const std::vector<HitInfo>& hits, SessionState& state) {
  if (!initW()) {
    endW();
    return {};
  }

  if (state.target_proc_info.pid == 0) {
    ImGui::TextUnformatted("No target, choose one...");
    endW();
    return {};
  }

  if (state.scan_type == ScanType::Hit || state.scan_type == ScanType::HitRescan ||
      state.scan_type == ScanType::HitFilter) {
    ImGui::TextUnformatted("Scanning in progress.");
    ImGui::NewLine();
    ImGui::ProgressBar(state.scan_progress);
    endW();
    return {};
  }

  if (hits.empty()) {
    ImGui::TextUnformatted("No hits!");
    endW();
    return {};
  }

  auto hit_table_action = drawHitTable(hits, state.target_info);

  PendingAction context_action{};
  if (selected_row_ >= hits.size()) selected_row_ = 0;
  if (selected_row_ >= 0 && static_cast<uint64_t>(selected_row_) <= hits.size()) {
    context_action = context.cycleContext<HitInfo>(
        hits[selected_row_].bytes_around, selected_row_, hits[selected_row_].value.size(), state.hit_refresh_seconds);
  }
  endW();

  if (!std::holds_alternative<std::monostate>(hit_table_action)) {
    return hit_table_action;
  }

  if (!std::holds_alternative<std::monostate>(context_action)) return context_action;

  return {};
}

PendingAction HitW::drawHitTable(const std::vector<HitInfo>& hits, const TargetInfo& target_info) {
  PendingAction return_action{};

  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 100.0F, 250.0F);
  if (!ImGui::BeginChild("hitstable", {0, avail - context_height})) {
    ImGui::EndChild();
    return {};
  }
  if (!ImGui::BeginTable("Hit Table", 5, ImGuiTableFlags_ScrollY)) {
    ImGui::EndChild();
    return {};
  }

  ImGui::TableSetupColumn("Index");
  ImGui::TableSetupColumn("Address");
  ImGui::TableSetupColumn("Value");
  ImGui::TableSetupColumn("Old Value");
  ImGui::TableSetupColumn("Status");
  ImGui::TableHeadersRow();

  ImGuiListClipper clipper;
  clipper.Begin(static_cast<int32_t>(hits.size()));
  while (clipper.Step()) {
    for (int32_t row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
      ImGui::TableNextRow();
      ImGui::PushID(static_cast<int32_t>(row));

      ImGui::TableNextColumn();

      ImGui::Text("%d", ImGui::TableGetRowIndex());

      ImGui::TableNextColumn();

      if (ImGui::Selectable("##selectable",
                            selected_row_ == row,
                            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
        selected_row_ = row;
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          is_editing_ = true;
          just_started_editing_ = true;
        }
      }
      if (ImGui::BeginPopupContextItem("hit_popup_menu")) {
        selected_row_ = row;
        if (ImGui::MenuItem("Add to Favourites"))
          return_action = Action::Favourite::Add{static_cast<uint64_t>(selected_row_)};
        if (ImGui::MenuItem("Copy address to clipboard")) {
          char buf[32];
          snprintf(buf, sizeof(buf), "0x%" PRIX64, hits[static_cast<uint64_t>(selected_row_)].location);
          ImGui::SetClipboardText(buf);
        }
        ImGui::EndPopup();
      }

      ImGui::SameLine();
      ImGui::Text("0x%" PRIX64, hits[static_cast<uint64_t>(row)].location);

      ImGui::TableNextColumn();
      bool just_started = just_started_editing_;
      if (is_editing_ && row == selected_row_) {
        if (just_started) {
          ImGui::SetKeyboardFocusHere();
          just_started_editing_ = false;
        }
        std::vector<uint8_t> tmpbuf = hits[static_cast<uint64_t>(row)].value;
        ImGui::PushStyleColor(ImGuiCol_NavHighlight, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        if (getTargetValue(target_info.target_type, tmpbuf, ImGuiInputTextFlags_EnterReturnsTrue)) {
          return_action = Action::Hit::Write{tmpbuf, static_cast<uint64_t>(row)};
          is_editing_ = false;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
        if (!just_started && (!ImGui::IsItemActive())) {
          is_editing_ = false;
          selected_row_ = -1;
        }
      } else
        printData(hits[static_cast<uint64_t>(row)].value, target_info.target_type);

      if (!hits[static_cast<uint64_t>(row)].previous_value.empty()) {
        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(169, 169, 169, 255));
        printData(hits[static_cast<uint64_t>(row)].previous_value, target_info.target_type);
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 75, 43, 255));
        ImGui::TextUnformatted(relativeStatusToStr(hits[static_cast<uint64_t>(row)].status).c_str());
        ImGui::PopStyleColor();
      }

      if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
        selected_row_ = -1;
      }

      ImGui::PopID();
    }
  }
  ImGui::EndTable();
  ImGui::EndChild();
  return return_action;
}
