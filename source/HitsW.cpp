#include "HitsW.h"
#include "LogW.h"
#include "display.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <atomic>
#include <cinttypes>
#include <cstdint>
#include <string>
#include <vector>

void HitsW::InitW() { ImGui::Begin("Hits"); }

void HitsW::EndW() { ImGui::End(); }

Action HitsW::CycleW(const std::vector<HitInfoT> &Hits,
                     const TargetInfoT &TargetInfo,
                     std::atomic<float> Progress) {
  InitW();
  if (Hits.empty()) {
    EndW();
    return {};
  }

  if (Progress != -1) {
    ImGui::Text("Scanning in progress.");
    ImGui::NewLine();
    ImGui::ProgressBar(Progress);
    ImGui::EndChild();
    return {};
  }

  Action return_val;

  auto return_1 = DrawHitTable(Hits, TargetInfo);

  bool disable_context_refresh = true;
  if (selected_row >= 0 && selected_row <= Hits.size()) {
    DrawContextMenu(Hits[selected_row]);
    disable_context_refresh = false;
  }
  AlignButtons();
  if (disable_context_refresh)
    ImGui::BeginDisabled();
  auto return_2 = DrawRefreshContextButton();
  if (disable_context_refresh)
    ImGui::EndDisabled();
  ImGui::SameLine();
  auto return_3 = DrawRefreshAllButton();

  EndW();

  if (return_1.Type != OpType::NONE)
    return return_1;
  if (return_2)
    return Action{OpType::REFRESH, DataType::HIT, selected_row};
  if (return_3)
    return Action{OpType::REFRESH_ALL, DataType::HIT, selected_row};

  return Action{OpType::NONE};
}

Action HitsW::DrawHitTable(const std::vector<HitInfoT> &Hits,
                           const TargetInfoT &TargetInfo) {
  Action ReturnAction;

  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1f, 100.0f, 250.0f);
  if (ImGui::BeginChild("hitstable", {0, avail - context_height})) {
    if (ImGui::BeginTable("Hit Table", 6, ImGuiTableFlags_ScrollY)) {

      ImGui::TableSetupColumn("##");
      ImGui::TableSetupColumn("Address");
      ImGui::TableSetupColumn("Value");
      ImGui::TableSetupColumn("Old Value");
      ImGui::TableSetupColumn("Status");
      ImGui::TableHeadersRow();

      ImGuiListClipper ListClipper;
      ListClipper.Begin(Hits.size());
      while (ListClipper.Step()) {
        for (uint32_t row = ListClipper.DisplayStart;
             row < ListClipper.DisplayEnd; ++row) {
          ImGui::TableNextRow();
          ImGui::PushID(row);

          ImGui::TableNextColumn();

          ImGui::Text("%d", ImGui::TableGetRowIndex());

          ImGui::TableNextColumn();

          if (ImGui::Selectable("##selectable", selected_row == row,
                                ImGuiSelectableFlags_SpanAllColumns |
                                    ImGuiSelectableFlags_AllowDoubleClick)) {
            selected_row = row;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
              IsEditing = true;
              JustStartedEditing = true;
            }
          } else {
            if (ImGui::BeginPopupContextItem("hit_context_menu")) {
              printf("ehhh\n");
              selected_row = row;
              if (ImGui::MenuItem("Add to Favourites")) {
                ReturnAction.Type = OpType::ADD_TO_FAVOURITES;
                ReturnAction.index = selected_row;
              }
              ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::Text("0x%" PRIX64, Hits[row].location);
          }

          ImGui::TableNextColumn();
          bool CancelEdit = true;
          if (IsEditing && row == selected_row) {
            if (JustStartedEditing) {
              ImGui::SetKeyboardFocusHere();
              JustStartedEditing = false;
              CancelEdit = false;
            }
            std::vector<uint8_t> tmpbuf(TargetInfo.value.size());
            ImGui::PushStyleColor(ImGuiCol_NavHighlight, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

            if (GetTargetValue(TargetInfo.TargetType, tmpbuf,
                               ImGuiInputTextFlags_EnterReturnsTrue)) {
              ReturnAction.newval = tmpbuf;
              ReturnAction.Type = OpType::EDIT;
              ReturnAction.WorkOn = DataType::HIT;
              ReturnAction.index = row;
              IsEditing = false;
              CancelEdit = true;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();
            if (CancelEdit && (!ImGui::IsItemActive())) {
              IsEditing = false;
              selected_row = -1;
            }
          } else
            ImGui::Text(
                "%s", ValToStr(Hits[row].value, TargetInfo.TargetType).c_str());
          if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
            selected_row = -1;
          }
          if (!Hits[row].previous_value.empty()) {
            ImGui::TableNextColumn();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(169, 169, 169, 255));
            ImGui::Text(
                "%s", ValToStr(Hits[row].previous_value, TargetInfo.TargetType)
                          .c_str());
            ImGui::PopStyleColor();

            ImGui::TableNextColumn();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 75, 43, 255));
            ImGui::Text("%s", RelativeStatusToStr(Hits[row].Status).c_str());
            ImGui::PopStyleColor();
          }
          ImGui::PopID();
        }
      }
      ImGui::EndTable();
    }
  }
  ImGui::EndChild();
  return ReturnAction;
}

void HitsW::DrawContextMenu(const HitInfoT Hit) {
  int constexpr BYTES_BEFORE = 32;
  int constexpr BYTES_AFTER = 32;
  int constexpr BYTES_PER_ROW = 16;

  if (Hit.bytes_around.size() !=
      Hit.value.size() + BYTES_BEFORE + BYTES_AFTER) {
    Log::Error("Hit " + std::to_string(Hit.location) +
               " is near a memory region and I have not implemented a way to "
               "reliably display bytes for that case. TODO later.");
    return;
  }

  for (int i = 0; i < Hit.bytes_around.size(); ++i) {
    ImGui::SameLine(0, 4);
    if (i % 32 == 0)
      ImGui::NewLine();
    else if (i % 8 == 0) {
      ImGui::Text(" ");
      ImGui::SameLine(0, 4);
    }
    // logic might be wrong let's see
    if (i >= BYTES_BEFORE && i + BYTES_AFTER < Hit.bytes_around.size()) {
      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 160, 100, 255));
      ImGui::Text("%02X", Hit.bytes_around[i]);
      ImGui::PopStyleColor();
    } else
      ImGui::Text("%02X", Hit.bytes_around[i]);
  }
}

bool HitsW::DrawRefreshAllButton() {
  float button_w = 150.0f;

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ImGui::GetContentRegionAvail().x - button_w);

  if (ImGui::Button("Refresh All Hits", {button_w, 0})) {
    return true;
  }
  return false;
}

bool HitsW::DrawRefreshContextButton() {
  float button_w = 150.0f;

  if (ImGui::Button("Refresh Context Hit", {button_w, 0})) {
    return true;
  }

  return false;
}

void HitsW::AlignButtons() {
  float button_h = ImGui::GetFrameHeight();
  float button_w = 150.0f;
  float current_h = ImGui::GetContentRegionAvail().y;

  if (current_h > button_h) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h);
  }

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       (ImGui::GetContentRegionAvail().x - button_w) / 2);
}
