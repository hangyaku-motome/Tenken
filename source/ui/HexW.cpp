#include "HexW.h"

#include <fcntl.h>
#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "misc/cpp/imgui_stdlib.h"
#include "utils.h"

constexpr int32_t context_bytes_before = 256;
constexpr int32_t context_bytes_after = 256;

constexpr int32_t bytes_per_row = 16;

bool HexW::initW() { return ImGui::Begin("Hex"); }

void HexW::endW() { ImGui::End(); }

void HexW::cycleW() {
  if (!enabled_) return;
  if (!initW()) {
    endW();
    return;
  }
  if (not scanner_->isAttached()) {
    ImGui::Text("No target...!");
    endW();
    return;
  }

  ImGui::InputScalar("Go to:", ImGuiDataType_U64, &search_address_, nullptr, nullptr, "%016lx");
  ImGui::SameLine();
  if (ImGui::Button("Go")) {
    shown_bytes_ = readAround(search_address_);
    current_address_ = search_address_;
  }
  ImGui::SameLine();
  if (ImGui::Button("Refresh")) {
    shown_bytes_ = readAround(current_address_);
  }
  if (!shown_bytes_.empty()) drawHexTable();

  endW();
}

void HexW::drawHexTable() {
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 50.0F, 150.0F);

  if (!ImGui::BeginChild("hextable", {0, avail - context_height})) return;

  if (!ImGui::BeginTable("Hex View", 18, ImGuiTableFlags_ScrollY)) {
    ImGui::EndChild();
    return;
  }

  ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 120.0F);
  for (uint32_t k = 0; k < bytes_per_row; ++k) ImGui::TableSetupColumn("00", ImGuiTableColumnFlags_WidthFixed, 25.0F);

  ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, 130.0F);

  for (uint64_t row = 0; row < (context_bytes_after + context_bytes_before) / bytes_per_row; ++row) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();

    uint64_t row_abs_address = current_address_ - context_bytes_before < 0
                                 ? (row * bytes_per_row)
                                 : current_address_ - context_bytes_before + (row * bytes_per_row);

    if (row_abs_address == search_address_) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 160, 100, 255));
    ImGui::Text("0x%" PRIX64, row_abs_address);
    if (row_abs_address == search_address_) ImGui::PopStyleColor();

    for (int32_t hex_column = 0; hex_column < bytes_per_row; ++hex_column) {
      ImGui::TableNextColumn();
      int32_t hex_index = row * bytes_per_row + hex_column;
      if (editing_index_ == hex_index) {
        std::string shown_str = hexToStr(shown_bytes_[hex_index]);
        ImGui::SetNextItemWidth(25.0F);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        if (ImGui::InputText(
                "##edit", &shown_str, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
          std::vector<uint8_t> write_value{static_cast<uint8_t>(strtoull(shown_str.c_str(), nullptr, 16))};
          scanner_->writeAdr(row_abs_address + hex_column, write_value);
          std::vector<uint8_t> read_byte = scanner_->readAdr(row_abs_address + hex_column, 1);
          shown_bytes_[hex_index] = read_byte[0];
        }
        if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) editing_index_ = -1;
        ImGui::PopStyleVar();
      } else {
        char label[16];
        snprintf(label, sizeof(label), "%02X##%d", shown_bytes_[hex_index], hex_index);
        if (ImGui::Selectable(label, false, ImGuiSelectableFlags_AllowDoubleClick)) editing_index_ = hex_index;
      }
    }

    ImGui::TableNextColumn();

    std::string print_buf;
    for (int32_t k = 0; k < bytes_per_row; ++k) {
      int32_t hex_index = row * bytes_per_row + k;
      if (std::isprint(static_cast<uint8_t>(shown_bytes_[hex_index])))
        print_buf += shown_bytes_[hex_index];
      else
        print_buf += '.';
    }
    ImGui::TextUnformatted(print_buf.c_str());
  }
  ImGui::EndTable();
  ImGui::EndChild();
}

// I do have something similar in DataInspector. probably might want to make it a function in scanner.

std::vector<uint8_t> HexW::readAround(const uint64_t adr) {
  uint64_t search_start = adr - context_bytes_before < 0 ? 0 : adr - context_bytes_before;
  return scanner_->readAdr(search_start, context_bytes_after + context_bytes_before);
}
