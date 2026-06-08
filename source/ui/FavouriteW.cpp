#include "FavouriteW.h"

#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <string>
#include <variant>

#include "display.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "types.h"
#include "utils.h"

bool FavouriteW::InitW() { return ImGui::Begin("Favourite"); }

void FavouriteW::EndW() { ImGui::End(); }

PendingAction FavouriteW::CycleW(const std::vector<FavouriteInfoT>& Favourites, SessionState& State) {
  if (!InitW()) {
    EndW();
    return {};
  }
  auto TableAction = DrawFavouriteTable(Favourites);

  PendingAction ContextAction;
  if (selected_row >= 0 && selected_row < static_cast<int64_t>(Favourites.size())) {
    auto cta = Context.CycleContext(
        static_cast<uint64_t>(selected_row), Favourites[static_cast<uint64_t>(selected_row)], State.favRefreshSeconds);
    ContextAction = Context.ResolveContextIntent(cta, false);
  }

  EndW();

  if (!std::holds_alternative<std::monostate>(TableAction)) return TableAction;

  if (!std::holds_alternative<std::monostate>(ContextAction)) return ContextAction;

  return {};
}

PendingAction FavouriteW::DrawFavouriteTable(const std::vector<FavouriteInfoT>& Favourites) {
  PendingAction ReturnAction;
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 100.0F, 250.0F);
  if (!ImGui::BeginChild("favouritestable", {0, avail - context_height})) return {};

  if (!ImGui::BeginTable("Favourites", 7, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
    ImGui::EndChild();
    return {};
  }
  ImGui::TableSetupColumn("Desc");
  ImGui::TableSetupColumn("Address");
  ImGui::TableSetupColumn("Value");
  ImGui::TableSetupColumn("Old Value");
  ImGui::TableSetupColumn("Status");
  ImGui::TableSetupColumn("Frozen");
  ImGui::TableSetupColumn("Type");

  for (uint64_t row = 0; row < Favourites.size(); row++) {
    ImGui::TableNextRow();
    ImGui::PushID(static_cast<int32_t>(row));

    ImGui::TableNextColumn();

    if (ImGui::Selectable("##selectable_all",
                          row == static_cast<uint64_t>(selected_row),
                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
      AllColumnChosen = true;
      selected_row = static_cast<int64_t>(row);
    } else if (ImGui::BeginPopupContextItem("favourite_menu")) {
      selected_row = static_cast<int64_t>(row);
      if (ImGui::MenuItem("Remove from Favourites")) ReturnAction = Action::removeFavourite{row};
      ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGui::Selectable("##selectable_desc",
                          IsEditingDesc,
                          ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap)) {
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
      if (ImGui::InputText(
              "##Description", &strbuf, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiSelectableFlags_AllowOverlap)) {
        ReturnAction = Action::descFavourite{static_cast<uint64_t>(selected_row), strbuf};
        IsEditingDesc = false;
        CancelEdit = true;
      }
      if (CancelEdit && !ImGui::IsItemActive()) {
        IsEditingDesc = false;
      }
    } else
      ImGui::TextUnformatted(Favourites[row].desc.c_str());

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::TableNextColumn();

    ImGui::Text("0x%" PRIX64, Favourites[row].location);

    ImGui::TableNextColumn();

    if (ImGui::Selectable("##selectable_value", IsEditingVal, ImGuiSelectableFlags_AllowDoubleClick)) {
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
      if (GetTargetValue(Favourites[row].type, newval_buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
        ReturnAction = Action::writeFavourite(row, newval_buf);
        IsEditingVal = false;
        CancelEdit = true;
      }
      if (CancelEdit && !ImGui::IsItemActive()) {
        IsEditingVal = false;
      }
    } else
      printData(Favourites[row].value, Favourites[row].type);
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::TableNextColumn();

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(169, 169, 169, 255));
    printData(Favourites[row].previous_value, Favourites[row].type);
    ImGui::PopStyleColor();

    ImGui::TableNextColumn();

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 75, 43, 255));
    ImGui::TextUnformatted(relativeStatusToStr(Favourites[row].status).c_str());
    ImGui::PopStyleColor();

    ImGui::TableNextColumn();

    bool Freeze = Favourites[row].frozen;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::Checkbox("##freeze", &Freeze);
    if (Freeze != Favourites[row].frozen) {
      ReturnAction = Action::isFreezeFavourite(row, Freeze);
    }
    ImGui::PopStyleVar();

    ImGui::TableNextColumn();

    ImGui::TextUnformatted(targetTypeToStr(Favourites[row].type).c_str());

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
