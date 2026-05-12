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

FavouriteWAction
FavouriteW::CycleW(const std::vector<FavouriteInfoT> &Favourites) {
  InitW();
  auto TableAction = DrawFavouriteTable(Favourites);

  FavouriteWAction ContextAction;
  if (selected_row >= 0 &&
      selected_row < static_cast<int64_t>(Favourites.size())) {
    bool SetRefresh =
        Favourites[static_cast<uint64_t>(selected_row)].auto_refresh_seconds !=
        -1;
    ContextAction = Context.CycleContext(
        static_cast<uint64_t>(selected_row),
        Favourites[static_cast<uint64_t>(selected_row)],
        Favourites[static_cast<uint64_t>(selected_row)].auto_refresh_seconds,
        SetRefresh);
  }

  EndW();

  if (TableAction.Type != OpType::NONE)
    return TableAction;

  if (ContextAction.Type != OpType::NONE)
    return ContextAction;

  return {};
}

FavouriteWAction
FavouriteW::DrawFavouriteTable(const std::vector<FavouriteInfoT> &Favourites) {

  FavouriteWAction ReturnAction;
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 100.0F, 250.0F);
  if (!ImGui::BeginChild("favouritestable", {0, avail - context_height}))
    return {};

  if (!ImGui::BeginTable("Favourites", 7,
                         ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
    return {};

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
    ImGui::PushID(static_cast<int32_t>(row));

    ImGui::TableNextColumn();

    if (ImGui::Selectable("##selectable_all",
                          row == static_cast<uint64_t>(selected_row),
                          ImGuiSelectableFlags_SpanAllColumns |
                              ImGuiSelectableFlags_AllowOverlap)) {
      AllColumnChosen = true;
      selected_row = static_cast<int64_t>(row);
    } else if (ImGui::BeginPopupContextItem("favourite_menu")) {
      selected_row = static_cast<int64_t>(row);
      if (ImGui::MenuItem("Remove from Favourites")) {
        ReturnAction.Type = OpType::REMOVE_FROM_FAVOURITES;
        ReturnAction.index = row;
      }
      ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGui::Selectable("##selectable_desc", IsEditingDesc,
                          ImGuiSelectableFlags_AllowDoubleClick |
                              ImGuiSelectableFlags_AllowOverlap)) {
      selected_row = static_cast<int64_t>(row);
      IsEditingDesc = true;
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
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
    if (IsEditingDesc && row == static_cast<uint64_t>(selected_row)) {
      if (JustStartedEditingDesc) {
        ImGui::SetKeyboardFocusHere();
        JustStartedEditingDesc = false;
        CancelEdit = false;
      }

      std::string strbuf;
      if (ImGui::InputText("##Description", &strbuf,
                           ImGuiInputTextFlags_EnterReturnsTrue |
                               ImGuiSelectableFlags_AllowOverlap)) {
        ReturnAction.index = selected_row;
        ReturnAction.Type = OpType::EDIT;
        ReturnAction.newname = strbuf;
        IsEditingDesc = false;
        CancelEdit = true;
      }
      if (CancelEdit && !ImGui::IsItemActive()) {
        IsEditingDesc = false;
      }
    } else
      ImGui::TextUnformatted(Favourites[row].Description.c_str());

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::TableNextColumn();

    ImGui::Text("0x%" PRIX64, Favourites[row].location);

    ImGui::TableNextColumn();

    if (ImGui::Selectable("##selectable_value", IsEditingVal,
                          ImGuiSelectableFlags_AllowDoubleClick)) {
      IsEditingVal = true;
      selected_row = static_cast<int64_t>(row);
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
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
    if (IsEditingVal && row == static_cast<uint64_t>(selected_row)) {
      if (JustStartedEditingVal) {
        ImGui::SetKeyboardFocusHere();
        JustStartedEditingVal = false;
        CancelEdit = false;
      }

      std::vector<uint8_t> newval_buf(Favourites[row].value.size());
      if (GetTargetValue(Favourites[row].TargetType, newval_buf,
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        ReturnAction.index = row;
        ReturnAction.Type = OpType::EDIT;
        ReturnAction.buf = newval_buf;
        IsEditingVal = false;
        CancelEdit = true;
      }
      if (CancelEdit && !ImGui::IsItemActive()) {
        IsEditingVal = false;
      }
    } else if (Favourites[row].TargetType != TargetTypeT::Invalid)
      ImGui::TextUnformatted(
          ValToStr(Favourites[row].value, Favourites[row].TargetType).c_str());
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::TableNextColumn();

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(169, 169, 169, 255));
    if (!Favourites[row].previous_value.empty())
      ImGui::TextUnformatted(
          ValToStr(Favourites[row].previous_value, Favourites[row].TargetType)
              .c_str());
    ImGui::PopStyleColor();

    ImGui::TableNextColumn();

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 75, 43, 255));
    if (Favourites[row].Status != RelativeStatus::UNSET)
      ImGui::TextUnformatted(
          RelativeStatusToStr(Favourites[row].Status).c_str());
    ImGui::PopStyleColor();

    ImGui::TableNextColumn();

    bool Freeze = Favourites[row].Frozen;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::Checkbox("##freeze", &Freeze);
    if (Freeze != Favourites[row].Frozen) {
      if (Freeze) {
        ReturnAction.Type = OpType::FREEZE;
        ReturnAction.index = row;
      } else {
        ReturnAction.Type = OpType::UNFREEZE;
        ReturnAction.index = row;
      }
    }
    ImGui::PopStyleVar();

    ImGui::TableNextColumn();

    ImGui::TextUnformatted(TargetTypeToStr(Favourites[row].TargetType).c_str());

    if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
      AllColumnChosen = false;
      selected_row = -1;
    }

    ImGui::PopID();
  }
  ImGui::EndTable();
  ImGui::EndChild();
  return ReturnAction;
}
