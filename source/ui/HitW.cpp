#include "HitW.h"

#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "ContextDisplay.h"
#include "display.h"
#include "imgui.h"
#include "types.h"
#include "utils.h"

bool HitW::initW() { return ImGui::Begin("Hits"); }

void HitW::endW() { ImGui::End(); }

PendingAction HitW::cycleW(const std::vector<HitInfoT>& hits, SessionState& state) {
  if (!initW()) {
    endW();
    return {};
  }

  if (state.target_proc_info.pid == 0) {
    ImGui::TextUnformatted("No target, choose one...");
    endW();
    return {};
  }

  if (state.is_scanning) {
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
  if (selected_row_ >= 0 && static_cast<uint64_t>(selected_row_) <= hits.size()) {
    auto ctr = context.cycleContext(
        static_cast<uint64_t>(selected_row_), hits[static_cast<uint64_t>(selected_row_)], state.hit_refresh_seconds);
    context_action = context.ResolveContextIntent(ctr, true);
  }
  endW();

  if (!std::holds_alternative<std::monostate>(hit_table_action)) {
    return hit_table_action;
  }

  if (!std::holds_alternative<std::monostate>(context_action)) return context_action;

  return {};
}

PendingAction HitW::drawHitTable(const std::vector<HitInfoT>& hits, const TargetInfoT& target_info) {
  PendingAction return_action{};

  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 100.0F, 250.0F);
  if (!ImGui::BeginChild("hitstable", {0, avail - context_height})) return {};
  if (!ImGui::BeginTable("Hit Table", 6, ImGuiTableFlags_ScrollY)) {
    ImGui::EndChild();
    return {};
  }

  ImGui::TableSetupColumn("##");
  ImGui::TableSetupColumn("Address");
  ImGui::TableSetupColumn("Value");
  ImGui::TableSetupColumn("Old Value");
  ImGui::TableSetupColumn("Status");
  ImGui::TableHeadersRow();

  ImGuiListClipper list_clipper;
  list_clipper.Begin(static_cast<int32_t>(hits.size()));
  while (list_clipper.Step()) {
    for (uint32_t row = static_cast<uint32_t>(list_clipper.DisplayStart);
         row < static_cast<uint32_t>(list_clipper.DisplayEnd);
         ++row) {
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
      } else {
        if (ImGui::BeginPopupContextItem("hit_popup_menu")) {
          selected_row_ = row;
          if (ImGui::MenuItem("Add to Favourites")) {
            return_action = Action::AddFavourite{static_cast<uint64_t>(selected_row_)};
          }
          if (ImGui::MenuItem("Copy address to clipboard")) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%016lx", hits[selected_row_].location);
            ImGui::SetClipboardText(buf);
          }
          ImGui::EndPopup();
        }
        ImGui::SameLine();
        ImGui::Text("0x%" PRIX64, hits[row].location);
      }

      ImGui::TableNextColumn();
      bool cancel_edit = true;
      if (is_editing_ && row == selected_row_) {
        if (just_started_editing_) {
          ImGui::SetKeyboardFocusHere();
          just_started_editing_ = false;
          cancel_edit = false;
        }
        std::vector<uint8_t> tmpbuf(target_info.value.size());
        tmpbuf = hits[row].value;
        ImGui::PushStyleColor(ImGuiCol_NavHighlight, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        if (GetTargetValue(target_info.target_type, tmpbuf, ImGuiInputTextFlags_EnterReturnsTrue)) {
          return_action = Action::WriteHit{row, tmpbuf};
          is_editing_ = false;
          cancel_edit = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
        if (cancel_edit && (!ImGui::IsItemActive())) {
          is_editing_ = false;
          selected_row_ = -1;
        }
      } else
        printData(hits[row].value, target_info.target_type);

      if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
        selected_row_ = -1;
      }
      if (!hits[row].previous_value.empty()) {
        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(169, 169, 169, 255));
        printData(hits[row].previous_value, target_info.target_type);
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 75, 43, 255));
        ImGui::TextUnformatted(relativeStatusToStr(hits[row].status).c_str());
        ImGui::PopStyleColor();
      }
      ImGui::PopID();
    }
  }
  ImGui::EndTable();
  ImGui::EndChild();
  return return_action;
}
