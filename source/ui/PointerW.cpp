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
    is_on_search_window_ = false;
    endW();
    return Action::StartPointerScan{.init_config = init_config};
  }

  ImGui::NewLine();
  ImGui::Text("Or...If you have a result to load in");
  if (ImGui::Button("Choose result")) file_browser_.Open();

  endW();
  return {};
}

void PointerW::cyclePointerListW(PointerList& pointer_list) {
  if (!pointer_list.is_file_open_) {
    ImGui::Text("No valid file loaded in. If you just tried to scan and are seeing this, this means there is a BUG and "
                "I couldn't parse the save file. Check logs for details (if there are any.)");
    if (ImGui::Button("Go back")) is_on_search_window_ = true;
    return endW();
  }
  ImGui::Text("%s", (std::to_string(pointer_list.total_chains_) + " chains in loaded file").c_str());
  if (ImGui::Button("Load in another result")) file_browser_.Open();

  if (!ImGui::BeginTable("Pointer Table", 2 + Pointer::max_depth, ImGuiTableFlags_ScrollY)) return;

  ImGui::TableSetupColumn("module");
  ImGui::TableSetupColumn("offset in module");
  // TODO: should be as much as biggest valid offsets in loaded file, not max depth.
  for (int i = 0; i < Pointer::max_depth; ++i)
    ImGui::TableSetupColumn((std::string("offset ") + std::to_string(i)).c_str());

  ImGui::TableHeadersRow();

  ImGuiListClipper clipper;
  clipper.Begin(pointer_list.total_chains_);

  while (clipper.Step()) {
    if (chains_.empty()) chains_ = pointer_list.getFrom(0, pointer_list.total_chains_);

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

  // also no.
  // the new system is making a pointer scan, automatically saving it. and then adding an option to "Test Pointer" in
  // file. from there it can automatically select the latest date one in share (maybe also give option to manually
  // choose path). When a pointer scan result is loaded in, it will try to resolve the pointers. uhhh for
  // that...........................................................I need to prompt the new location of the value in
  // "Test Pointer". and when a chain DOES eventually lead to that address, we can keep it. otherwise remove. NOT SURE
  // WHERE TO SAVE EXACTLY. But afterwards we show result in PointerW, and give choice to save. they can label each
  // pointer, aaand we can also add a checkbox next to each entry to save or not.
}

PendingAction PointerW::cycleW(const SessionState& state, PointerList& pointer_list) {
  if (not enabled_) return {};
  if (!initW()) {
    endW();
    return {};
  };

  if (file_browser_.HasSelected()) {
    printf("this is done");
    pointer_list.openFile(file_browser_.GetSelected());
    is_on_search_window_ = false;
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
  if (is_on_search_window_) {
    return cycleSearchW();
  };

  cyclePointerListW(pointer_list);

  return {};
}
