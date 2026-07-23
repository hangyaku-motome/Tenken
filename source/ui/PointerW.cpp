#include "PointerW.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <X11/Xdefs.h>

#include <iostream>
#include <string>

#include "types.h"

bool PointerW::initW() { return ImGui::Begin("Pointer"); }

void PointerW::endW() { ImGui::End(); }

PendingAction PointerW::cycleSearchW() {
  ImGui::InputScalar("Points near:", ImGuiDataType_U64, &init_config.address, nullptr, nullptr, "%016lx");

  ImGui::NewLine();

  ImGui::Text("don't change unless you can't find the one you are looking for.");
  ImGui::InputScalar("Scan before:", ImGuiDataType_S32, &init_config.info.search_before, nullptr, nullptr, nullptr);
  ImGui::InputScalar("Scan after:", ImGuiDataType_S32, &init_config.info.search_after, nullptr, nullptr, nullptr);
  ImGui::InputScalar("depth:", ImGuiDataType_U8, &init_config.info.scan_depth, nullptr, nullptr, nullptr);

  if (ImGui::Button("Scan")) {
    is_on_search_window_ = false;
    endW();
    return Action::StartPointerScan{.init_config = init_config};
  }

  ImGui::NewLine();
  ImGui::Text("Or...If you have a result to load in");

  // So...After having a pair of pointer lists (either by scanning once then loading in the other one or loading in
  // both), on second windows (pointer listW) there needs to be an option of compare against.

  endW();
  return {};
}

void PointerW::cyclePointerListW(PointerList& pointer_list) {
  if (!pointer_list.is_file_open_) {
    ImGui::Text("No valid file loaded in. If you just tried to scan and are seeing this, this means there is a BUG and "
                "I couldn't parse the save file. Check logs for details (if there are any.)");
    if (ImGui::Button("Load in another result")) file_browser_.Open();
    return endW();
  }
  ImGui::Text("%s", (std::to_string(pointer_list.total_chains_) + " chains in loaded file").c_str());
  if (ImGui::Button("Load in another result")) file_browser_.Open();

  if (!ImGui::BeginTable("Pointer Table", 2 + Pointer::max_depth, ImGuiTableFlags_ScrollY)) return;

  ImGui::TableSetupColumn("module");
  ImGui::TableSetupColumn("offset in module");
  for (int i = 0; i < Pointer::max_depth; ++i)
    ImGui::TableSetupColumn((std::string("offset ") + std::to_string(i)).c_str());

  ImGui::TableHeadersRow();

  printf("THIS IS SIZE OF CHAINS %lu\n\n\n", chains_.size());
  printf("THIS IS TOTAL SIZE OF CHAINS %lu\n\n\n", pointer_list.total_chains_);

  ImGuiListClipper clipper;
  clipper.Begin(pointer_list.total_chains_);

  while (clipper.Step()) {
    if (chains_.empty()) chains_ = pointer_list.get_from(0, pointer_list.total_chains_);

    std::cout << "tried to get from to in chains " << clipper.DisplayStart << " "
              << clipper.DisplayEnd - clipper.DisplayStart << "\n";

    for (uint64_t i = 0; i + clipper.DisplayStart < clipper.DisplayEnd; ++i) {
      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::Text("%s", chains_[i].module_name.c_str());

      ImGui::TableNextColumn();
      ImGui::Text("%s", std::to_string(chains_[i].offset_in_module).c_str());

      for (int k = 0; k < Pointer::max_depth; ++k) {
        ImGui::TableNextColumn();
        if (k < chains_[i].offsets.size())
          ImGui::Text("%s", std::to_string(chains_[i].offsets[k]).c_str());
        else
          ImGui::Text("-");
      }
    }
  }
  ImGui::EndTable();
  return endW();

  // this many chains found.
  // Info:: The result has been saved. You should choose another pointer scan result if you have one.
  // If not, run the pointer scan on another instance of the program to get another one. Afterwards you can compare
  // them.
  //
}

PendingAction PointerW::cycleW(const SessionState& state, PointerList& pointer_list) {
  if (not enabled_) return {};
  if (!initW()) {
    endW();
    return {};
  };

  file_browser_.Display();

  if (state.scan_type == ScanType::Pointer) {
    ImGui::Text("Scanning...");
    endW();
    return {};
  }
  if (is_on_search_window_) {
    return cycleSearchW();
  };

  cyclePointerListW(pointer_list);
  return {};
}
