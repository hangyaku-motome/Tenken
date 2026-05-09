#include "Favourite.h"
#include "display.h"
#include "imgui.h"
#include "types.h"
#include <cinttypes>

void FavouriteW::InitW() { ImGui::Begin("Favourite"); }

void FavouriteW::EndW() { ImGui::End(); }

Action FavouriteW::CycleW(std::vector<FavouriteInfoT> &Favourites,
                          const TargetTypeT &TargetType) {
  InitW();
  if (ImGui::BeginTable("Favourites", 6,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
    ImGui::TableSetupColumn("Desc");
    ImGui::TableSetupColumn("Address");
    ImGui::TableSetupColumn("Value");
    ImGui::TableSetupColumn("Old Value");
    ImGui::TableSetupColumn("Relative Status");
    ImGui::TableSetupColumn("Frozen");
    ImGui::TableHeadersRow();

    for (uint64_t i = 0; i < Favourites.size(); i++) {
      printf("hi\n");
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (!Favourites[i].Description.empty())
        ImGui::Text("%s", Favourites[i].Description.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("0x%" PRIX64, Favourites[i].location);
      ImGui::TableNextColumn();
      ImGui::Text("%s", ValToStr(Favourites[i].value, TargetType).c_str());
      ImGui::TableNextColumn();
      if (!Favourites[i].previous_value.empty())
        ImGui::Text("%s",
                    ValToStr(Favourites[i].previous_value, TargetType).c_str());
      ImGui::TableNextColumn();
      if (Favourites[i].Status != RelativeStatus::UNSET)
        ImGui::Text("%s", Favourites[i].Status);
      ImGui::TableNextColumn();
      ImGui::Checkbox("##freeze", &Favourites[i].Frozen);
    }
    ImGui::EndTable();
  }
  EndW();
  return Action{OpType::NONE};
}