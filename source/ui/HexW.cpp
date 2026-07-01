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

constexpr int32_t CONTEXT_BYTES_BEFORE = 256;
constexpr int32_t CONTEXT_BYTES_AFTER = 256;

constexpr int32_t BYTES_PER_ROW = 16;

bool HexW::InitW() { return ImGui::Begin("Hex"); }

void HexW::EndW() { ImGui::End(); }

void HexW::CycleW() {
  if (!enabled_) return;
  if (!InitW()) {
    EndW();
    return;
  }

  ImGui::InputScalar("Go to:", ImGuiDataType_U64, &searchAddress_, nullptr, nullptr, "%016lx");
  ImGui::SameLine();
  if (ImGui::Button("Go")) {
    shownBytes_ = readAround(searchAddress_);
    currentAddress_ = searchAddress_;
  }
  ImGui::SameLine();
  if (ImGui::Button("Refresh")) {
    shownBytes_ = readAround(currentAddress_);
  }
  if (!shownBytes_.empty()) drawHexTable();

  EndW();
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
  for (uint32_t k = 0; k < BYTES_PER_ROW; ++k) ImGui::TableSetupColumn("00", ImGuiTableColumnFlags_WidthFixed, 25.0F);

  ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, 130.0F);

  for (uint64_t row = 0; row < (CONTEXT_BYTES_AFTER + CONTEXT_BYTES_BEFORE) / BYTES_PER_ROW; ++row) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();

    uint64_t rowAbsAddress = currentAddress_ - CONTEXT_BYTES_BEFORE < 0
                                 ? (row * BYTES_PER_ROW)
                                 : currentAddress_ - CONTEXT_BYTES_BEFORE + (row * BYTES_PER_ROW);

    if (rowAbsAddress == searchAddress_) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 160, 100, 255));
    ImGui::Text("0x%" PRIX64, rowAbsAddress);
    if (rowAbsAddress == searchAddress_) ImGui::PopStyleColor();

    for (int32_t hex_column = 0; hex_column < BYTES_PER_ROW; ++hex_column) {
      ImGui::TableNextColumn();
      int32_t hex_index = row * BYTES_PER_ROW + hex_column;
      if (editing_index == hex_index) {
        std::string shown_str = hexToStr(shownBytes_[hex_index]);
        ImGui::SetNextItemWidth(25.0F);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        if (ImGui::InputText(
                "##edit", &shown_str, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
          std::vector<uint8_t> write_value{static_cast<uint8_t>(strtoull(shown_str.c_str(), nullptr, 16))};
          scanner_->writeAdr(rowAbsAddress + hex_column, write_value);
          std::vector<uint8_t> read_byte = scanner_->readAdr(rowAbsAddress + hex_column, 1);
          shownBytes_[hex_index] = read_byte[0];
        }
        if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) editing_index = -1;
        ImGui::PopStyleVar();
      } else {
        char label[16];
        snprintf(label, sizeof(label), "%02X##%d", shownBytes_[hex_index], hex_index);
        if (ImGui::Selectable(label, false, ImGuiSelectableFlags_AllowDoubleClick)) editing_index = hex_index;
      }
    }

    ImGui::TableNextColumn();

    std::string printbuf;
    for (int32_t k = 0; k < BYTES_PER_ROW; ++k) {
      int32_t hex_index = row * BYTES_PER_ROW + k;
      if (std::isprint(static_cast<uint8_t>(shownBytes_[hex_index])))
        printbuf += shownBytes_[hex_index];
      else
        printbuf += '.';
    }
    ImGui::TextUnformatted(printbuf.c_str());
  }
  ImGui::EndTable();
  ImGui::EndChild();
}

// I do have something similar in DataInspector. probably might want to make it a function in scanner.

std::vector<uint8_t> HexW::readAround(const uint64_t adr) {
  uint64_t search_start = adr - CONTEXT_BYTES_BEFORE < 0 ? 0 : adr - CONTEXT_BYTES_BEFORE;
  return scanner_->readAdr(search_start, CONTEXT_BYTES_AFTER + CONTEXT_BYTES_BEFORE);
}
