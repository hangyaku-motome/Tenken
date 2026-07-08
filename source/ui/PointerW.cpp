#include "PointerW.h"

#include <imgui.h>

#include <algorithm>
#include <cinttypes>
#include <string>

#include "types.h"

using Action::StartPointerScan;

bool PointerW::initW() { return ImGui::Begin("Pointer"); }

void PointerW::endW() { ImGui::End(); }

// TODO:settings don't work right now. all of those pointer configs.
PendingAction PointerW::cycleSearchW() {
  ImGui::InputScalar("Points near:", ImGuiDataType_U64, &tmp_target_adr_, nullptr, nullptr, "%016lx");

  ImGui::NewLine();

  ImGui::Text("don't change unless you can't find the one you are looking for.");
  ImGui::InputScalar("Scan before:", ImGuiDataType_S32, &tmp_scan_before_, nullptr, nullptr, nullptr);
  ImGui::InputScalar("Scan after:", ImGuiDataType_S32, &tmp_scan_after_, nullptr, nullptr, nullptr);
  ImGui::InputScalar("depth:", ImGuiDataType_U8, &depth_limit_, nullptr, nullptr, nullptr);

  if (ImGui::Button("Scan")) {
    is_on_search_window_ = false;
    endW();
    return StartPointerScan{.search_for = tmp_target_adr_,
                            .depth = depth_limit_,
                            .bytes_before = tmp_scan_before_,
                            .bytes_after = tmp_scan_after_};
  }
  endW();
  return {};
}

void PointerW::cyclePointerListW(const std::vector<PointerChain>& chains) {
  ImGui::Text("I am supposed to show pointers.");
  ImGui::Text("%s", std::to_string(chains.size()).c_str());

  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 100.0F, 250.0F);
  if (!ImGui::BeginChild("pointer_child", {0, avail - context_height})) {
    ImGui::EndChild();
    return endW();
  };

  int32_t column_count = 3 + depth_limit_;
  if (!ImGui::BeginTable("Pointer Table", column_count, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
    ImGui::EndChild();
    return endW();
  }

  ImGui::TableSetupColumn("##");
  ImGui::TableSetupColumn("module");
  ImGui::TableSetupColumn("offset in module");
  for (int32_t i = 0; i < depth_limit_; ++i)
    ImGui::TableSetupColumn((std::string("offset") + std::to_string(i + 1)).c_str());
  ImGui::TableHeadersRow();

  ImGuiListClipper list_clipper;
  list_clipper.Begin(static_cast<int32_t>(chains.size()));
  while (list_clipper.Step()) {
    for (uint32_t row = static_cast<uint32_t>(list_clipper.DisplayStart);
         row < static_cast<uint32_t>(list_clipper.DisplayEnd);
         ++row) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(std::to_string(row).c_str());

      ImGui::TableNextColumn();
      ImGui::Text("%s", chains[row].module_name.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("0x%lx", chains[row].offset_in_module);
      for (int32_t i = 0; i < depth_limit_; ++i) {
        ImGui::TableNextColumn();
        if (i < chains[row].offsets.size())
          ImGui::Text("0x%ld", chains[row].offsets[i]);
        else
          ImGui::TextUnformatted("-");
      }
    }
  }
  ImGui::EndTable();
  ImGui::EndChild();
  if (ImGui::Button("Go back:")) is_on_search_window_ = true;
  return endW();
}

PendingAction PointerW::cycleW(const std::vector<PointerChain>& chains, ScanType scan_type) {
  if (not enabled_) return {};
  if (!initW()) {
    endW();
    return {};
  };

  if (scan_type == ScanType::Pointer) {
    ImGui::Text("Scanning...");
    endW();
    return {};
  }
  if (chains.empty() && not is_on_search_window_) {
    ImGui::Text("No pointers.");
    if (ImGui::Button("Go back")) is_on_search_window_ = true;
    endW();
    return {};
  }

  if (is_on_search_window_) {
    return cycleSearchW();
  };

  cyclePointerListW(chains);
  return {};
}
