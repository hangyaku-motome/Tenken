#include "PointerW.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <X11/Xdefs.h>

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
    endW();
    return Action::Scan::StartPointer{.init_config = init_config};
  }

  ImGui::NewLine();
  ImGui::Text("Or...If you have a result to load in");
  if (ImGui::Button("Choose result")) file_browser_.Open();

  endW();
  return {};
}

void PointerW::cyclePointerListW(PointerList& pointer_list) {
  if (pointer_list.failed()) {
    ImGui::Text("I couldn't parse the save file. Check logs for details (if there are any (hopefully there is )).");
    if (ImGui::Button("Go back")) pointer_list.close();
    return endW();
  }

  if (pointer_list.total_chains_ == 0) chains_.clear();

  // TODO: GOTTA GET IN PIECES WHEN IT'S A LOT
  if (chains_.empty()) chains_ = pointer_list.getFrom(0, pointer_list.total_chains_);
  if (chains_.empty()) {
    ImGui::Text("I couldn't extract the pointers for some reason. maybe log has details.");
    if (ImGui::Button("Load in another result")) file_browser_.Open();
    ImGui::SameLine();
    if (ImGui::Button("Go back")) {
      pointer_list.close();
      return endW();
    }
    return endW();
  }

  ImGui::Text("%s", (std::to_string(pointer_list.total_chains_) + " chains in loaded file").c_str());
  if (ImGui::Button("Load in another result")) file_browser_.Open();
  ImGui::SameLine();
  if (ImGui::Button("Go back")) {
    pointer_list.close();
    return endW();
  }

  if (!ImGui::BeginTable("Pointer Table", 2 + Pointer::max_depth, ImGuiTableFlags_ScrollY)) return;

  ImGui::TableSetupColumn("module");
  ImGui::TableSetupColumn("offset in module");

  // TODO: should be as much as biggest valid offsets in loaded file, not max depth....but calculating that might be
  // tricky.
  for (int i = 0; i < Pointer::max_depth; ++i)
    ImGui::TableSetupColumn((std::string("offset ") + std::to_string(i)).c_str());

  ImGui::TableHeadersRow();

  ImGuiListClipper clipper;
  clipper.Begin(chains_.size());

  while (clipper.Step()) {
    for (int32_t i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
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
}

// SOME stuff to handle: I try to open from somewhere else, it neeeds to show me that screen instead of the search one
// still. basically if a request was made to open a file, it should switch to that window.
PendingAction PointerW::cycleW(const SessionState& state, PointerList& pointer_list) {
  if (not enabled_) return {};
  if (!initW()) {
    endW();
    return {};
  };

  if (file_browser_.HasSelected()) {
    pointer_list.openFile(file_browser_.GetSelected());
    chains_.clear();
    file_browser_.Close();
  }

  file_browser_.Display();

  // TODO: make it possible to do hit scanning and pointer scanning at the same time.
  if (state.scan_type == ScanType::Pointer) {
    ImGui::Text("Scanning...");
    endW();
    return {};
  }

  if (!pointer_list.isOpen()) return cycleSearchW();

  cyclePointerListW(pointer_list);
  return {};
}
