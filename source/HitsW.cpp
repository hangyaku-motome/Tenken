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

void HitsW::InitW(WindowInfoT HitsWindow, ImGuiWindowFlags Flags) {
  ImGui::SetNextWindowPos(ImVec2(HitsWindow.XPos, HitsWindow.YPos));
  ImGui::SetNextWindowSize(ImVec2(HitsWindow.W, HitsWindow.H));

  ImGui::Begin("Hits", nullptr, Flags);
}

void HitsW::EndW() { ImGui::End(); }

void HitsW::CycleW(WindowInfoT HitsWindow, ImGuiWindowFlags Flags,
                   const std::vector<HitInfoT> &Hits,
                   const TargetInfoT &TargetInfo) {
  InitW(HitsWindow, Flags);
  DrawHitTable(Hits, TargetInfo);
  if (selected_row > 0 && selected_row <= Hits.size()) {
    DrawContextMenu(Hits[selected_row]);
  }
  EndW();
}

// add refresh button.
// Returns chosen row.
void HitsW::DrawHitTable(const std::vector<HitInfoT> &Hits,
                         const TargetInfoT &TargetInfo) {
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.3f, 150.0f, 350.0f);
  if (ImGui::BeginChild("hitstable", {0, avail - context_height},
                        !ImGuiChildFlags_Borders)) {
    printf("%f\n", avail - context_height);
    if (ImGui::BeginTable("Hit Table", 4, ImGuiTableFlags_ScrollY)) {
      printf("table started\n");
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

// I really really don't wanna have this read bytes live,
// but the chosen one IS the only one that needs their bytes updated. everything
// else is value. should I really be updating the bytes of every single hit?
// seems redundant. value on the other hand is a must.
// But then there would be times where "bytes around" are just really stale and
// their freshenss depends on being explicitly called by this one object. Is
// that okay?

// verdict:
// we should NOT refresh everything every x miliseconds or something by default.
// Since this is live, data WILL get stale. However it will represent a stable
// snapshot of what "was". We *should* have a button to refresh values and tag
// them and display their previous value. we should also be able to have the
// user be able to choose automatic refresh. Even when opening byte screen, only
// refresh ON DEMAND OR explicit automatic refresh. We CANNOT just arbitarily
// refresh them.
// ***

void HitsW::DrawContextMenu(const HitInfoT Hit) {
  int constexpr BYTES_BEFORE = 32;
  int constexpr BYTES_AFTER = 32;
  int constexpr BYTES_PER_ROW = 16;

  if (Hit.bytes_around.size() !=
      Hit.value.size() + BYTES_BEFORE + BYTES_AFTER) {
    printf("bytes around size: %zu, other stuff: %zu\n",
           Hit.bytes_around.size(),
           Hit.value.size() + BYTES_AFTER + BYTES_BEFORE);
    Log::Error("Hit " + std::to_string(Hit.location) +
               " is near a memory region and I have not implemented a way to "
               "reliably display bytes for that case. TODO later.");
    return;
  }

  // 16 bytes per row with spacing after the 8th seems fine...In my mind.

  for (int i = 0; i < Hit.bytes_around.size(); ++i) {
    if (i % 16 == 0)
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

    ImGui::SameLine(0, 4);
  }
}
