#include "PointerW.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <X11/Xdefs.h>

#include <string>

#include "display.h"
#include "types.h"

bool PointerW::initW() { return ImGui::Begin("Pointer"); }

void PointerW::endW() { ImGui::End(); }

PendingAction PointerW::cycleSearchW(const State& state) {
  ImGui::InputScalar("Points near:", ImGuiDataType_U64, &init_config_.address, nullptr, nullptr, "%016lx");

  ImGui::NewLine();

  ImGui::Text("don't change unless you can't find the one you are looking for.");
  ImGui::InputScalar("Scan before:", ImGuiDataType_S32, &init_config_.info.search_before, nullptr, nullptr, nullptr);
  ImGui::InputScalar("Scan after:", ImGuiDataType_S32, &init_config_.info.search_after, nullptr, nullptr, nullptr);
  ImGui::InputScalar("depth:", ImGuiDataType_U8, &init_config_.info.scan_depth, nullptr, nullptr, nullptr);

  ImGui::NewLine();
  ImGui::NewLine();
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Target type is inferred from latest done scan type by default, so it is likely correct.\nChange "
                      "it if it's wrong.");
  ImGui::SameLine();

  if (init_config_.target_type == TargetType::invalid) init_config_.target_type = state.target_info.target_type;
  auto new_type = getTargetType(init_config_.target_type);
  if (new_type != TargetType::invalid) init_config_.target_type = new_type;
  ImGui::BeginDisabled(init_config_.target_type == TargetType::invalid);
  if (ImGui::Button("Scan")) {
    endW();
    return Action::Scan::StartPointer{.init_config = init_config_};
  }
  ImGui::EndDisabled();

  ImGui::NewLine();
  ImGui::Text("Or...If you have a result to load in");
  if (ImGui::Button("Choose result")) file_browser_.Open();

  endW();
  return {};
}

void PointerW::cyclePointerListW(PointerList& pointer_list) {
  if (pointer_list.getStatus() == -1) {
    ImGui::Text("I couldn't parse the save file. Check logs for details (if there are any (hopefully there is )).");
    if (ImGui::Button("Go back")) pointer_list.close();
    return endW();
  }

  if (pointer_list.just_opened_) {
    chains_.clear();
    pointer_list.just_opened_ = false;
  }

  // TODO: GOTTA GET IN PIECES WHEN IT'S A LOT
  if (chains_.empty()) chains_ = pointer_list.getFrom(0, pointer_list.total_chains_);
  if (chains_.empty()) {
    ImGui::Text(
        "I couldn't extract the pointers for some reason. maybe log has details. Or there are no chains in file?");
    if (ImGui::Button("Load in another result")) file_browser_.Open();
    ImGui::SameLine();
    if (ImGui::Button("Go back")) {
      pointer_list.close();
      return endW();
    }
    return endW();
  }

  ImGui::Text("%lu chains in loaded file", pointer_list.total_chains_);
  ImGui::Text("%s", (std::to_string(pointer_list.total_chains_) + " chains in loaded file").c_str());
  ImGui::SameLine();
  if (pointer_list.getSaveIndex() == 0)
    ImGui::Text("First scan results.");
  else
    ImGui::Text("%ddth filter", pointer_list.getSaveIndex());
  ;
  if (ImGui::Button("Load in another result")) file_browser_.Open();
  ImGui::SameLine();
  if (ImGui::Button("Go back")) {
    pointer_list.close();
    return endW();
  }

  if (!ImGui::BeginTable("Pointer Table", 2 + Pointer::MaxDepth, ImGuiTableFlags_ScrollY)) return;

  ImGui::TableSetupColumn("module");
  ImGui::TableSetupColumn("offset in module");

  for (int32_t i = 0; i < pointer_list.getDepth(); ++i)
    ImGui::TableSetupColumn((std::string("offset ") + std::to_string(i + 1)).c_str());

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

      for (int32_t k = 0; k < pointer_list.getDepth(); ++k) {
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

PendingAction PointerW::cycleW(const State& state, PointerList& pointer_list) {
  if (not enabled_) return {};
  if (!initW()) {
    endW();
    return {};
  };

  file_browser_.Display();

  if (file_browser_.HasSelected()) {
    pointer_list.openFile(file_browser_.GetSelected());
    file_browser_.Close();
  }

  // TODO: make it possible to do hit scanning and pointer scanning at the same time.
  if (state.scan_type == ScanType::Pointer) {
    ImGui::Text("Scanning...");
    endW();
    return {};
  }

  if (pointer_list.getStatus() == 0) return cycleSearchW(state);

  cyclePointerListW(pointer_list);
  return {};
}
