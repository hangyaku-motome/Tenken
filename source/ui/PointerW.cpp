#include "PointerW.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <format>
#include <string>

#include "display.h"
#include "types.h"
#include "utils.h"

bool PointerW::initW() { return ImGui::Begin("Pointer"); }

void PointerW::endW() { ImGui::End(); }

PendingAction PointerW::cycleSearchW(const State& state) {
  PendingAction action{};

  ImGui::InputScalar("Points near:", ImGuiDataType_U64, &init_config_.address, nullptr, nullptr, "%016lx");

  ImGui::NewLine();

  ImGui::Text("don't change unless you can't find the one you are looking for.");
  ImGui::InputScalar("Scan before:", ImGuiDataType_S32, &init_config_.info.search_before);
  ImGui::InputScalar("Scan after:", ImGuiDataType_S32, &init_config_.info.search_after);
  ImGui::InputScalar("depth:", ImGuiDataType_U8, &init_config_.info.scan_depth);

  ImGui::NewLine();
  ImGui::NewLine();
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Target type is inferred from latest done scan type by default, so it is likely correct.\nChange "
                      "it if it's wrong.");
  ImGui::SameLine();

  if (init_config_.target_type == TargetType::invalid) init_config_.target_type = state.target_info.target_type;
  auto new_type = getTargetType(init_config_.target_type);

  if (init_config_.target_type == TargetType::string || init_config_.target_type == TargetType::aob) {
    ImGui::InputScalar("length", ImGuiDataType_U8, &init_config_.target_size);
  }

  ImGui::BeginDisabled(
      (init_config_.target_type == TargetType::invalid) ||
      ((init_config_.target_type == TargetType::aob || init_config_.target_type == TargetType::string) &&
       init_config_.target_size == 0));
  if (ImGui::Button("Scan")) {
    if (new_type != TargetType::aob && new_type != TargetType::string)
      init_config_.target_size = targetTypeToSize(init_config_.target_type);
    action = Action::Scan::StartPointer{.init_config = init_config_};
  }
  ImGui::EndDisabled();

  ImGui::NewLine();
  ImGui::Text("Or...If you have a result to load in");
  if (ImGui::Button("Choose result")) file_browser_.Open();

  endW();
  return action;
}

PendingAction PointerW::cyclePointerListW(PointerList& pointer_list) {
  PendingAction action{};
  if (pointer_list.getStatus() == -1) {
    ImGui::Text("I couldn't parse the save file. Check logs for details (if there are any (hopefully there is )).");
    if (ImGui::Button("Go back")) pointer_list.close();
    endW();
    return {};
  }

  if (pointer_list.just_opened_) {
    chains_.clear();
    pointer_list.just_opened_ = false;
  }

  // TODO: GOTTA GET IN PIECES WHEN IT'S A LOT
  // ehhhhh will do later
  if (chains_.empty()) chains_ = pointer_list.getFrom(0, pointer_list.total_chains_);
  if (chains_.empty()) {
    ImGui::Text("I couldn't extract the pointers. Maybe log has details. Or there are no chains in file?");
    if (ImGui::Button("Load in another result")) file_browser_.Open();
    ImGui::SameLine();
    if (ImGui::Button("Go back")) {
      pointer_list.close();
      endW();
      return {};
    }
    endW();
    return {};
  }

  ImGui::Text("%lu chains in loaded file", pointer_list.total_chains_);
  ImGui::SameLine();
  if (pointer_list.getSaveIndex() == 0)
    ImGui::Text("First scan results.");
  else
    ImGui::Text("filter number: %d", pointer_list.getSaveIndex());
  ;
  if (ImGui::Button("Load in another result")) file_browser_.Open();
  ImGui::SameLine();
  if (ImGui::Button("Go back")) {
    pointer_list.close();
    endW();
    return {};
  }

  ImGui::BeginDisabled(favourite_index == -1);
  if (ImGui::Button("Add selected chain to favourites.")) {
    action = Action::Favourite::AddChain{.index = static_cast<uint64_t>(favourite_index),
                                         .target_size = pointer_list.getTargetSize(),
                                         .target_type = pointer_list.getTargetType()};
  }
  ImGui::EndDisabled();

  if (!ImGui::BeginTable("Pointer Table", 2 + pointer_list.getDepth(), ImGuiTableFlags_ScrollY)) return {};

  ImGui::TableSetupColumn("module");
  ImGui::TableSetupColumn("offset in module");

  for (int32_t i = 0; i < pointer_list.getDepth(); ++i)
    ImGui::TableSetupColumn(std::format("offset {}", i + 1).c_str());

  ImGui::TableHeadersRow();

  ImGuiListClipper clipper;
  clipper.Begin(chains_.size());

  while (clipper.Step()) {
    for (int32_t i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
      ImGui::PushID(i);
      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::Text("%s", chains_[i].module_name.c_str());

      ImGui::SameLine();
      if (ImGui::Selectable(
              "##", favourite_index == i, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
        favourite_index = i;

      ImGui::TableNextColumn();
      ImGui::Text("%lu", chains_[i].offset_in_module);

      for (int32_t k = 0; k < pointer_list.getDepth(); ++k) {
        ImGui::TableNextColumn();
        if (k < chains_[i].offsets.size())
          ImGui::Text("%li", chains_[i].offsets[k]);
        else
          ImGui::Text("-");
      }
      ImGui::PopID();
    }
  }
  ImGui::EndTable();
  endW();
  return action;
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

  return cyclePointerListW(pointer_list);
}
