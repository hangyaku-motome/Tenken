#include "MapPopUp.h"

#include <imgui.h>

#include <cinttypes>
#include <string>

#include "LogW.h"
#include "types.h"
#include "utils.h"

// TODO: Maybe move popup names or window names to a single const for each windows/pop up?

void MapsPopUp::initPopUp() {
  ImGui::OpenPopup("Regions List");
  clicked_ = false;
}

void MapsPopUp::updateRegions() {
  regions_ = scanner_->getMapRegions();
  Log::Info(std::to_string(regions_.size()) + " regions found.");
}

void MapsPopUp::renderTable() {
  if (!ImGui::BeginTable("Regions", 5)) return;

  for (auto const& region : regions_) {
    ImGui::TableNextRow();
    ImGui::PushID(region.start);

    ImGui::TableNextColumn();
    bool is_active = active_addresses_.contains(region.start);
    if (ImGui::Checkbox("##", &is_active)) {
      if (is_active)
        active_addresses_.insert(region.start);
      else
        active_addresses_.erase(region.start);
    }

    ImGui::TableNextColumn();
    ImGui::Text("0x%" PRIX64, region.start);

    ImGui::TableNextColumn();
    ImGui::Text("0x%" PRIX64, region.end);
    if (ImGui::BeginPopupContextItem("map_popup_menu")) {
      if (ImGui::MenuItem("Copy start address to clipboard")) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%016lx", region.start);
        ImGui::SetClipboardText(buf);
      }
      if (ImGui::MenuItem("Copy end address to clipboard")) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%016lx", region.end);
        ImGui::SetClipboardText(buf);
      }
      ImGui::EndPopup();
    }

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(region.name.data(), region.name.data() + region.name.size());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(mapTypeToStr(region.type).c_str());

    ImGui::PopID();
  }

  ImGui::EndTable();
}

void MapsPopUp::CyclePUp(std::vector<MapInfoT>& ActiveRegions) {
  if (clicked_) initPopUp();
   if (ActiveRegions.empty() && scanner_->isAttached()) {
    updateRegions();
    applyDefaultFilters();
    ActiveRegions = buildFilteredMap();
  }

 if (!ImGui::BeginPopupModal("Regions List", nullptr, popup_flags)) return;

  if (not scanner_->isAttached()) {
    ImGui::Text("Nothing! Obviously! No targets! Go choose one.");
    if (ImGui::Button("Leave in utter shame")) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    return;
  }

  ImGui::TextUnformatted("You can toggle these filters on and off to choose specific memory regions to scan. Most of "
                         "the time the defaults are the right ones.");

  if (ImGui::Checkbox("code", &filter_.code)) {
    toggleFilter(MapType::MAIN_EXEC_CODE, filter_.code);
    toggleFilter(MapType::SHARED_LIB_CODE, filter_.code);
  }
  ImGui::SameLine();
  if (ImGui::Checkbox("anon", &filter_.anon)) {
    toggleFilter(MapType::ANON, filter_.anon);
  }
  ImGui::SameLine();
  if (ImGui::Checkbox("heap", &filter_.heap)) {
    toggleFilter(MapType::HEAP, filter_.heap);
  }
  ImGui::SameLine();
  if (ImGui::Checkbox("lib data", &filter_.lib_data)) {
    toggleFilter(MapType::SHARED_LIB_DATA, filter_.lib_data);
  }
  ImGui::SameLine();
  if (ImGui::Checkbox("main exec data", &filter_.main_exec_data)) {
    toggleFilter(MapType::MAIN_EXEC_DATA, filter_.main_exec_data);
  }
  ImGui::SameLine();
  if (ImGui::Checkbox("read_only_const", &filter_.read_only_const)) {
    toggleFilter(MapType::MAIN_EXEC_CONST, filter_.read_only_const);
    toggleFilter(MapType::SHARED_LIB_CONST, filter_.read_only_const);
  }

  renderTable();

  if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
  ImGui::SameLine();
  if (ImGui::Button("Refresh")) {
    updateRegions();
    applyDefaultFilters();
  }

  ImGui::BeginDisabled(active_addresses_.size() == 0);
  if (ImGui::Button("Apply Filter")) {
    ActiveRegions = buildFilteredMap();
    ImGui::CloseCurrentPopup();
  }
  ImGui::EndDisabled();

  ImGui::EndPopup();

  return;
}

void MapsPopUp::applyDefaultFilters() {
  for (auto const& region : regions_) {
    switch (region.type) {
      case MapType::MAIN_EXEC_DATA:
      case MapType::ANON:
      case MapType::HEAP:
        active_addresses_.insert(region.start);
        continue;
      case MapType::MAIN_EXEC_CODE:
      case MapType::MAIN_EXEC_CONST:
      case MapType::SHARED_LIB_CODE:
      case MapType::SHARED_LIB_DATA:
      case MapType::SHARED_LIB_CONST:
      case MapType::KERNEL_PAGES:
      case MapType::STACK:
      case MapType::UNREADABLE:
      case MapType::UNSET:
        continue;
    }
  }
}

void MapsPopUp::toggleFilter(MapType type, bool enable) {
  for (auto const& region : regions_) {
    if (region.type != type) continue;
    if (not enable)  // I didn't even know "not" existed in C++. I just tried and it worked LOL. I'll keep it like this
                     // even if it makes less sense for now cause I just wanna see it.
      active_addresses_.erase(region.start);
    else
      active_addresses_.insert(region.start);
  }
}

std::vector<MapInfoT> MapsPopUp::buildFilteredMap() {
  std::vector<MapInfoT> maps;
  for (const auto& region : regions_) {
    if (active_addresses_.contains(region.start)) maps.push_back(region);
  }
  Log::Info(std::to_string(maps.size()) + " regions in filtered list.");
  return maps;
}
