#include "Favourite.h"
#include "display.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "types.h"
#include <algorithm>
#include <cinttypes>
#include <cstdint>

void FavouriteW::InitW() { ImGui::Begin("Favourite"); }

void FavouriteW::EndW() { ImGui::End(); }

Action FavouriteW::CycleW(const std::vector<FavouriteInfoT> &Favourites) {
  InitW();
  auto TableAction = DrawFavouriteTable(Favourites);

  Action ContextAction;
  bool disable_context_refresh = true;
  if (selected_row >= 0 && selected_row <= Favourites.size()) {
    bool SetRefresh =
        Favourites[selected_row].auto_refresh_seconds == -1 ? false : true;
    ContextAction = Context.CycleContext(
        selected_row, Favourites[selected_row],
        Favourites[selected_row].auto_refresh_seconds, SetRefresh);
  }

  EndW();

  if (TableAction.Type != OpType::NONE)
    return TableAction;

  if (ContextAction.Type != OpType::NONE) {
    ContextAction.WorkOn = DataType::FAVOURITE;
    ContextAction.index = selected_row;
    return ContextAction;
  }

  return {}; // unsure if we should return none or {} (I think default handles
             // that right?)
}

Action
FavouriteW::DrawFavouriteTable(const std::vector<FavouriteInfoT> &Favourites) {

  Action ReturnAction;
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1f, 100.0f, 250.0f);
  if (ImGui::BeginChild("favouritestable", {0, avail - context_height})) {
    if (ImGui::BeginTable("Favourites", 7,
                          ImGuiTableFlags_Resizable |
                              ImGuiTableFlags_ScrollY)) {
      ImGui::TableSetupColumn("Desc");
      ImGui::TableSetupColumn("Address");
      ImGui::TableSetupColumn("Value");
      ImGui::TableSetupColumn("Old Value");
      ImGui::TableSetupColumn("Status");
      ImGui::TableSetupColumn("Frozen");
      ImGui::TableSetupColumn("Type");
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
          if (GetTargetValue(Favourites[row].TargetType, newval_buf,
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
          ImGui::Text(
              "%s", ValToStr(Favourites[row].value, Favourites[row].TargetType)
                        .c_str());
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(169, 169, 169, 255));
        if (!Favourites[row].previous_value.empty())
          ImGui::Text("%s", ValToStr(Favourites[row].previous_value,
                                     Favourites[row].TargetType)
                                .c_str());
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 75, 43, 255));
        if (Favourites[row].Status != RelativeStatus::UNSET)
          ImGui::Text("%s",
                      RelativeStatusToStr(Favourites[row].Status).c_str());
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();

        bool Freeze = Favourites[row].Frozen;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
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
        ImGui::PopStyleVar();

        ImGui::TableNextColumn();

        ImGui::Text("%s", TargetTypetoStr(Favourites[row].TargetType).c_str());

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