#include "HitsW.h"
#include "LogW.h"
#include "display.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <string>

void HitsW::InitW() {
  ImGui::SetNextWindowPos(ImVec2(Window.XPos, Window.YPos));
  ImGui::SetNextWindowSize(ImVec2(Window.W, Window.H));

  ImGui::Begin("Hits", nullptr, Window.flags);
}

void HitsW::EndW() { ImGui::End(); }

std::string HitsW::CycleW(const std::vector<HitInfoT> &Hits,
                          const TargetInfoT &TargetInfo) {
  InitW();
  if (Hits.empty()) {
    EndW();
    return "";
  }
  std::string return_val = "";

  DrawHitTable(Hits, TargetInfo);

  bool disable_context_refresh = true;
  if (selected_row >= 0 && selected_row <= Hits.size()) {
    DrawContextMenu(Hits[selected_row]);
    disable_context_refresh = false;
  }
  AlignButtons();
  if (disable_context_refresh)
    ImGui::BeginDisabled();
  std::string return_1 = DrawRefreshContextButton();
  if (disable_context_refresh)
    ImGui::EndDisabled();
  ImGui::SameLine();
  std::string return_2 = DrawRefreshAllButton();

  return_val = return_1.empty() ? return_2 : return_1;

  EndW();

  return return_val;
}

void HitsW::DrawHitTable(const std::vector<HitInfoT> &Hits,
                         const TargetInfoT &TargetInfo) {
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1f, 100.0f, 250.0f);
  if (ImGui::BeginChild("hitstable", {0, avail - context_height},
                        !ImGuiChildFlags_Borders)) {
    if (ImGui::BeginTable("Hit Table", 4, ImGuiTableFlags_ScrollY)) {
      ImGuiListClipper ListClipper;
      ListClipper.Begin(Hits.size());
      while (ListClipper.Step()) {
        for (uint32_t row = ListClipper.DisplayStart;
             row < ListClipper.DisplayEnd; ++row) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%d", ImGui::TableGetRowIndex() + 1);
          ImGui::TableNextColumn();
          char location_buf[16];
          std::snprintf(location_buf, sizeof(location_buf), "0x%" PRIXPTR,
                        Hits[row].location);

          if (ImGui::Selectable(location_buf, selected_row == row,
                                ImGuiSelectableFlags_SpanAllColumns)) {
            selected_row = row;
          }
          ImGui::TableNextColumn();
          ImGui::Text("%s", HitToStr(Hits[row].value, TargetInfo).c_str());
        }
      }
      ImGui::EndTable();
    }
    ImGui::EndChild();
  }
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
      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 220, 100, 255));
      ImGui::Text("%02X", Hit.bytes_around[i]);
      ImGui::PopStyleColor();
    } else
      ImGui::Text("%02X", Hit.bytes_around[i]);
  }
}

std::string HitsW::DrawRefreshAllButton() {
  float button_w = 150.0f;

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ImGui::GetContentRegionAvail().x - button_w);

  if (ImGui::Button("Refresh All Hits", {button_w, 0})) {
    return "refresh all";
  }
  return "";
}

std::string HitsW::DrawRefreshContextButton() {
  float button_w = 150.0f;

  if (ImGui::Button("Refresh Context Hit", {button_w, 0})) {
    return "refresh context";
  }

  return "";
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