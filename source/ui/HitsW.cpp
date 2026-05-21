#include "HitsW.h"
#include "ContextDisplay.h"
#include "display.h"
#include "imgui.h"
#include "types.h"
#include "utils.h"
#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <string>
#include <variant>
#include <vector>

bool HitsW::InitW() { return ImGui::Begin("Hits"); }

void HitsW::EndW() { ImGui::End(); }

PendingAction HitsW::CycleW(const std::vector<HitInfoT> &Hits, SessionState &State) {
  if (!InitW()) {
    EndW();
    return {};
  }

  if (!State.TargetChosen) {
    ImGui::TextUnformatted("No target, choose one...");
    EndW();
    return {};
  }

  if (State.IsScanning) {
    ImGui::TextUnformatted("Scanning in progress.");
    ImGui::NewLine();
    ImGui::ProgressBar(State.ScanProgress);
    EndW();
    return {};
  }

  if (Hits.empty()) {
    ImGui::TextUnformatted("No hits!");
    EndW();
    return {};
  }

  auto HitTableAction = DrawHitTable(Hits, State.TargetInfo);

  PendingAction ContextAction{};
  if (selected_row >= 0 && static_cast<uint64_t>(selected_row) <= Hits.size()) {
    auto ctr = Context.CycleContext(static_cast<uint64_t>(selected_row),
                                    Hits[static_cast<uint64_t>(selected_row)], State.hitRefreshSeconds);
    ContextAction = Context.ResolveContextIntent(ctr, true);
  }
  EndW();

  if (!std::holds_alternative<std::monostate>(HitTableAction)) {
    return HitTableAction;
  }

  if (!std::holds_alternative<std::monostate>(ContextAction))
    return ContextAction;

  return {};
}

PendingAction HitsW::DrawHitTable(const std::vector<HitInfoT> &Hits, const TargetInfoT &TargetInfo) {
  PendingAction ReturnAction{};

  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 100.0F, 250.0F);
  if (!ImGui::BeginChild("hitstable", {0, avail - context_height}))
    return {};
  if (!ImGui::BeginTable("Hit Table", 6, ImGuiTableFlags_ScrollY))
    return {};

  ImGui::TableSetupColumn("##");
  ImGui::TableSetupColumn("Address");
  ImGui::TableSetupColumn("Value");
  ImGui::TableSetupColumn("Old Value");
  ImGui::TableSetupColumn("Status");
  ImGui::TableHeadersRow();

  ImGuiListClipper ListClipper;
  ListClipper.Begin(static_cast<int32_t>(Hits.size()));
  while (ListClipper.Step()) {
    for (uint32_t row = static_cast<uint32_t>(ListClipper.DisplayStart);
         row < static_cast<uint32_t>(ListClipper.DisplayEnd); ++row) {
      ImGui::TableNextRow();
      ImGui::PushID(static_cast<int32_t>(row));

      ImGui::TableNextColumn();

      ImGui::Text("%d", ImGui::TableGetRowIndex());

      ImGui::TableNextColumn();

      if (ImGui::Selectable("##selectable", selected_row == row,
                            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
        selected_row = row;
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          IsEditing = true;
          JustStartedEditing = true;
        }
      } else {
        if (ImGui::BeginPopupContextItem("hit_popup_menu")) {
          selected_row = row;
          if (ImGui::MenuItem("Add to Favourites")) {
            ReturnAction = Action::addFavourite{static_cast<uint64_t>(selected_row)};
          }
          if (ImGui::MenuItem("Copy address to clipboard")) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%016lx", Hits[selected_row].location);
            ImGui::SetClipboardText(buf);
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
        tmpbuf = Hits[row].value;
        ImGui::PushStyleColor(ImGuiCol_NavHighlight, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        if (GetTargetValue(TargetInfo.TargetType, tmpbuf, ImGuiInputTextFlags_EnterReturnsTrue)) {
          ReturnAction = Action::writeHit{row, tmpbuf};
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
        printData(Hits[row].value, TargetInfo.TargetType);

      if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
        selected_row = -1;
      }
      if (!Hits[row].previous_value.empty()) {
        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(169, 169, 169, 255));
        printData(Hits[row].previous_value, TargetInfo.TargetType);
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 75, 43, 255));
        ImGui::TextUnformatted(relativeStatusToStr(Hits[row].status).c_str());
        ImGui::PopStyleColor();
      }
      ImGui::PopID();
    }
  }
  ImGui::EndTable();
  ImGui::EndChild();
  return ReturnAction;
}
