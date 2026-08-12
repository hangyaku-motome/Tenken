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

bool FavouriteW::initW() { return ImGui::Begin("Favourite"); }

void FavouriteW::endW() { ImGui::End(); }

PendingAction FavouriteW::cycleW(const std::vector<FavouriteInfo>& favourites, SessionState& state) {
  if (!initW()) {
    endW();
    return {};
  }
  auto table_action = drawFavouriteTable(favourites);

  PendingAction context_action;
  if (selected_row_ >= 0 && selected_row_ < static_cast<int64_t>(favourites.size())) {
    context_action = context.cycleContext<FavouriteInfo>(favourites[selected_row_].bytes_around,
                                                         selected_row_,
                                                         favourites[selected_row_].value.size(),
                                                         state.hit_refresh_seconds);
  }

  endW();

  if (!std::holds_alternative<std::monostate>(table_action)) return table_action;

  if (!std::holds_alternative<std::monostate>(context_action)) return context_action;

  return {};
}

PendingAction FavouriteW::drawFavouriteTable(const std::vector<FavouriteInfo>& favourites) {
  PendingAction return_action;
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 100.0F, 250.0F);
  if (!ImGui::BeginChild("favouritestable", {0, avail - context_height})) {
    ImGui::EndChild();
    return {};
  }

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
  ImGui::TableHeadersRow();

  for (uint64_t row = 0; row < favourites.size(); row++) {
    ImGui::TableNextRow();
    ImGui::PushID(static_cast<int32_t>(row));

    ImGui::TableNextColumn();
    if (ImGui::Selectable("##selectable_all",
                          row == static_cast<uint64_t>(selected_row_),
                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
      selected_row_ = static_cast<int64_t>(row);
    }
    if (ImGui::BeginPopupContextItem("favourite_menu")) {
      selected_row_ = static_cast<int64_t>(row);
      if (ImGui::MenuItem("Remove from Favourites")) return_action = Action::Favourite::Remove{row};
      if (ImGui::MenuItem("Copy address to clipboard")) {
        char buf[32];
        snprintf(buf, sizeof(buf), "0x%" PRIX64, favourites[static_cast<uint64_t>(selected_row_)].location);
        ImGui::SetClipboardText(buf);
      }
      ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGui::Selectable("##selectable_desc",
                          is_editing_desc_ && row == static_cast<uint64_t>(selected_row_),
                          ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap)) {
      selected_row_ = static_cast<int64_t>(row);
      is_editing_desc_ = true;
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) just_started_editing_desc_ = true;
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_NavHighlight, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    bool cancel_edit = true;
    if (is_editing_desc_ && row == static_cast<uint64_t>(selected_row_)) {
      if (just_started_editing_desc_) {
        ImGui::SetKeyboardFocusHere();
        just_started_editing_desc_ = false;
        cancel_edit = false;
      }

      std::string strbuf = favourites[row].desc;
      if (ImGui::InputText("##Description", &strbuf, ImGuiInputTextFlags_EnterReturnsTrue)) {
        return_action = Action::Favourite::Desc{strbuf, static_cast<uint64_t>(selected_row_)};
        is_editing_desc_ = false;
        cancel_edit = true;
      }
      if (cancel_edit && !ImGui::IsItemActive()) {
        is_editing_desc_ = false;
      }
    } else
      ImGui::TextUnformatted(favourites[row].desc.c_str());

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::TableNextColumn();

    ImGui::Text("0x%" PRIX64, favourites[row].location);

    ImGui::TableNextColumn();

    if (ImGui::Selectable("##selectable_value",
                          is_editing_val_ && row == static_cast<uint64_t>(selected_row_),
                          ImGuiSelectableFlags_AllowDoubleClick)) {
      is_editing_val_ = true;
      selected_row_ = static_cast<int64_t>(row);
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        is_editing_val_ = true;
        just_started_editing_val_ = true;
      }
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_NavHighlight, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    cancel_edit = true;
    if (is_editing_val_ && row == static_cast<uint64_t>(selected_row_)) {
      if (just_started_editing_val_) {
        ImGui::SetKeyboardFocusHere();
        just_started_editing_val_ = false;
        cancel_edit = false;
      }

      std::vector<uint8_t> newval_buf = favourites[row].value;
      if (getTargetValue(favourites[row].type, newval_buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
        return_action = Action::Favourite::Write(newval_buf, row);
        is_editing_val_ = false;
        cancel_edit = true;
      }
      if (cancel_edit && !ImGui::IsItemActive()) {
        is_editing_val_ = false;
      }
    } else
      printData(favourites[row].value, favourites[row].type);
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(169, 169, 169, 255));
    printData(favourites[row].previous_value, favourites[row].type);
    ImGui::PopStyleColor();

    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 75, 43, 255));
    ImGui::TextUnformatted(relativeStatusToStr(favourites[row].status).c_str());
    ImGui::PopStyleColor();

    ImGui::TableNextColumn();
    bool freeze = favourites[row].frozen;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::Checkbox("##freeze", &freeze);
    if (freeze != favourites[row].frozen) {
      return_action = Action::Favourite::IsFreeze(row, freeze);
    }
    ImGui::PopStyleVar();

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(targetTypeToStr(favourites[row].type).c_str());

    ImGui::PopID();
  }
  if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
    selected_row_ = -1;
  }
  ImGui::EndTable();
  ImGui::EndChild();
  return return_action;
}
