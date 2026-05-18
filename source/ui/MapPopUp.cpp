#include "MapPopUp.h"
#include "LogW.h"
#include "types.h"
#include <cinttypes>
#include <imgui.h>
#include <string>

void MapsPopUp::InitPopUp() {
  refresh_ = true;
  clicked_ = false;
  printf("made it here\n");
  ImGui::OpenPopup("Regions List");
  printf("ehwe\n");
}

void MapsPopUp::UpdateRegions(std::vector<MapInfoT> regions) {
  regions_ = regions;
  refresh_ = false;
  Log::Info(std::to_string(regions.size()) + " regions found.");
}

void MapsPopUp::CyclePUp() {
  if (clicked_)
    InitPopUp();

  if (!ImGui::BeginPopupModal("Regions List", nullptr,
                              ImGuiWindowFlags_AlwaysAutoResize |
                                  ImGuiWindowFlags_AlwaysVerticalScrollbar |
                                  ImGuiWindowFlags_HorizontalScrollbar))
    return;

  ImGui::TextUnformatted("List regions here:");

  if (!ImGui::BeginTable("Regions", 3))
    return;

  for (auto const &region : regions_) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("0x%" PRIX64, region.start);

    ImGui::TableNextColumn();
    ImGui::Text("0x%" PRIX64, region.end);

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(region.name.data(),
                           region.name.data() + region.name.size());
  }

  ImGui::EndTable();

  if (ImGui::Button("Cancel")) {
    ImGui::CloseCurrentPopup();
  }

  if (ImGui::Button("Refresh")) {
    refresh_ = true;
  }

  ImGui::EndPopup();
}