#include "Favourite.h"
#include "LogW.h"
#include "display.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "types.h"
#include <algorithm>
#include <cinttypes>
#include <cstdint>

void FavouriteW::InitW() { ImGui::Begin("Favourite"); }

void FavouriteW::EndW() { ImGui::End(); }

Action FavouriteW::CycleW(const std::vector<FavouriteInfoT> &Favourites,
                          const TargetTypeT &TargetType) {
  InitW();
  auto ActionType = DrawFavouriteTable(Favourites, TargetType);

  bool disable_context_refresh = true;
  if (selected_row >= 0 && selected_row <= Favourites.size()) {
    DrawContextMenu(Favourites[selected_row]);
    disable_context_refresh = false;
  }
  AlignButtons();
  if (disable_context_refresh)
    ImGui::BeginDisabled();
  auto return_1 = DrawRefreshContextButton();
  if (disable_context_refresh)
    ImGui::EndDisabled();
  EndW();

  if (return_1)
    return Action{OpType::REFRESH, DataType::FAVOURITE, selected_row};

  return ActionType;
}

Action
FavouriteW::DrawFavouriteTable(const std::vector<FavouriteInfoT> &Favourites,
                               const TargetTypeT &TargetType) {

  Action ReturnAction;
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1f, 100.0f, 250.0f);
  if (ImGui::BeginChild("favouritestable", {0, avail - context_height})) {
    if (ImGui::BeginTable("Favourites", 6,
                          ImGuiTableFlags_Resizable |
                              ImGuiTableFlags_ScrollY)) {
      ImGui::TableSetupColumn("Desc");
      ImGui::TableSetupColumn("Address");
      ImGui::TableSetupColumn("Value");
      ImGui::TableSetupColumn("Old Value");
      ImGui::TableSetupColumn("Status");
      ImGui::TableSetupColumn("Frozen");
      ImGui::TableHeadersRow();

      for (uint64_t row = 0; row < Favourites.size(); row++) {
        ImGui::TableNextRow();
        ImGui::PushID(row);

        ImGui::TableNextColumn();

        if (ImGui::Selectable("##selectable_all", row == selected_row,
                              ImGuiSelectableFlags_SpanAllColumns |
                                  ImGuiSelectableFlags_AllowOverlap)) {
          AllColumnChosen = true;
          selected_row = row;
          printf("all of itt\n");
        }
        ImGui::SameLine();
        if (ImGui::Selectable("##selectable_desc", IsEditingDesc == true,
                              ImGuiSelectableFlags_AllowDoubleClick |
                                  ImGuiSelectableFlags_AllowOverlap)) {
          selected_row = row;
          IsEditingDesc = true;
          if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            printf("click!\n");
            IsEditingDesc = true;
            JustStartedEditingDesc = true;
          }
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_NavHighlight, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        bool CancelEdit = true;
        if (IsEditingDesc && row == selected_row) {
          if (JustStartedEditingDesc) {
            printf("just started editing.\n");
            ImGui::SetKeyboardFocusHere();
            JustStartedEditingDesc = false;
            CancelEdit = false;
          }

          std::string description_buf;
          if (ImGui::InputText("##Description", &description_buf,
                               ImGuiInputTextFlags_EnterReturnsTrue |
                                   ImGuiSelectableFlags_AllowOverlap)) {
            ReturnAction.index = selected_row;
            ReturnAction.newname = description_buf;
            ReturnAction.WorkOn = DataType::FAVOURITE;
            ReturnAction.Type = OpType::CHANGE_NAME;
            printf("pressed enter so IsEditing and CancelEdit are false");
            IsEditingDesc = false;
            CancelEdit = true;
          }
          if (CancelEdit && !ImGui::IsItemActive()) {
            printf("cancelll\n");
            IsEditingDesc = false;
          }
        } else
          ImGui::Text("%s", Favourites[row].Description.c_str());
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        ImGui::TableNextColumn();

        ImGui::Text("0x%" PRIX64, Favourites[row].location);

        ImGui::TableNextColumn();

        if (ImGui::Selectable("##selectable_value", IsEditingVal == true,
                              ImGuiSelectableFlags_AllowDoubleClick)) {
          IsEditingVal = true;
          selected_row = row;
          if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            printf("click!\n");
            IsEditingVal = true;
            JustStartedEditingVal = true;
          }
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_NavHighlight, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        CancelEdit = true;
        if (IsEditingVal && row == selected_row) {
          if (JustStartedEditingVal) {
            printf("just started editing11.\n");
            ImGui::SetKeyboardFocusHere();
            JustStartedEditingVal = false;
            CancelEdit = false;
          }

          std::vector<uint8_t> newval_buf(Favourites[row].value.size());
          if (GetTargetValue(TargetType, newval_buf,
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            ReturnAction.WorkOn = DataType::FAVOURITE;
            ReturnAction.index = row;
            ReturnAction.Type = OpType::EDIT;
            ReturnAction.newval = newval_buf;
            printf("pressed enter so IsEditing and CancelEdit are false11\n");
            IsEditingVal = false;
            CancelEdit = true;
          }
          if (CancelEdit && !ImGui::IsItemActive()) {
            printf("cancelll11xasda\n");
            IsEditingVal = false;
          }
        } else
          ImGui::Text("%s",
                      ValToStr(Favourites[row].value, TargetType).c_str());
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(169, 169, 169, 255));
        if (!Favourites[row].previous_value.empty())
          ImGui::Text(
              "%s",
              ValToStr(Favourites[row].previous_value, TargetType).c_str());
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 75, 43, 255));
        if (Favourites[row].Status != RelativeStatus::UNSET)
          ImGui::Text("%s",
                      RelativeStatusToStr(Favourites[row].Status).c_str());
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();

        bool Freeze = Favourites[row].Frozen;
        ImGui::Checkbox("##freeze", &Freeze);
        if (Freeze != Favourites[row].Frozen) {
          if (Freeze) {
            ReturnAction.Type = OpType::FREEZE;
            ReturnAction.WorkOn = DataType::FAVOURITE;
            ReturnAction.index = row;
          } else {
            ReturnAction.Type = OpType::UNFREEZE;
            ReturnAction.WorkOn = DataType::FAVOURITE;
            ReturnAction.index = row;
          }
        }
        if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
          AllColumnChosen = false;
          selected_row = -1;
        }

        ImGui::PopID();
      }
      ImGui::EndTable();
    }
  }
  ImGui::EndChild();
  return ReturnAction;
}

bool FavouriteW::DrawRefreshContextButton() {
  float button_w = 150.0f;

  if (ImGui::Button("Refresh Context Hit", {button_w, 0})) {
    return true;
  }

  return false;
}

void FavouriteW::AlignButtons() {
  float button_h = ImGui::GetFrameHeight();
  float button_w = 150.0f;
  float current_h = ImGui::GetContentRegionAvail().y;

  if (current_h > button_h) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h);
  }

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       (ImGui::GetContentRegionAvail().x - button_w) / 2);
}

void FavouriteW::DrawContextMenu(const FavouriteInfoT Favourite) {
  int constexpr BYTES_BEFORE = 32;
  int constexpr BYTES_AFTER = 32;
  int constexpr BYTES_PER_ROW = 16;

  if (Favourite.bytes_around.size() !=
      Favourite.value.size() + BYTES_BEFORE + BYTES_AFTER) {
    Log::Error("Favourite " + std::to_string(Favourite.location) +
               " is near a memory region and I have not implemented a way to "
               "reliably display bytes for that case. TODO later.");
    return;
  }

  for (int i = 0; i < Favourite.bytes_around.size(); ++i) {
    ImGui::SameLine(0, 4);
    if (i % 32 == 0)
      ImGui::NewLine();
    else if (i % 8 == 0) {
      ImGui::Text(" ");
      ImGui::SameLine(0, 4);
    }
    // logic might be wrong let's see
    if (i >= BYTES_BEFORE && i + BYTES_AFTER < Favourite.bytes_around.size()) {
      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 160, 100, 255));
      ImGui::Text("%02X", Favourite.bytes_around[i]);
      ImGui::PopStyleColor();
    } else
      ImGui::Text("%02X", Favourite.bytes_around[i]);
  }
}