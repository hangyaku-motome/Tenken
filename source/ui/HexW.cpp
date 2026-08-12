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

  ImGui::InputScalar("Go to:", ImGuiDataType_U64, &search_address_buf, nullptr, nullptr, "%016lx");
  ImGui::SameLine();
  if (ImGui::Button("Go")) {
    read_region_ = scanner_->readAround(search_address_buf, config_.bytes_before, config_.bytes_after);
    current_address_ = search_address_buf;
  }
  ImGui::SameLine();
  if (ImGui::Button("Refresh")) {
    read_region_ = scanner_->readAround(search_address_buf, config_.bytes_before, config_.bytes_after);
    current_address_ = search_address_buf;
  }

  ImGui::SameLine();
  if (ImGui::Button("Config")) ImGui::OpenPopup("Config");

  if (!read_region_.read_bytes.empty()) drawHexTable();

  drawConfigPopup();

  endW();
}

void HexW::drawHexTable() {
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 50.0F, 150.0F);

  if (!ImGui::BeginChild("hextable", {0, avail - context_height})) {
    ImGui::EndChild();
    return;
  }

  if (!ImGui::BeginTable("Hex View", 18, ImGuiTableFlags_ScrollY)) {
    ImGui::EndChild();
    return;
  }

  ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 120.0F);
  for (uint32_t k = 0; k < config_.bytes_per_row; ++k)
    ImGui::TableSetupColumn("00", ImGuiTableColumnFlags_WidthFixed, 25.0F);

  ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, 130.0F);

  for (uint64_t row = 0; row < read_region_.read_bytes.size() / config_.bytes_per_row; ++row) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();

    uint64_t row_abs_address = current_address_ + read_region_.offset_from_adr + (row * config_.bytes_per_row);

    if (row_abs_address == search_address_buf) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 160, 100, 255));
    ImGui::Text("0x%" PRIX64, row_abs_address);
    if (row_abs_address == search_address_buf) ImGui::PopStyleColor();

    ImGui::TableNextColumn();

    auto res = drawRow(read_region_.read_bytes, row * config_.bytes_per_row);

    if (res != -1) {
      uint8_t byte = static_cast<uint8_t>(strtoull(edit_str_buf.c_str(), nullptr, 16));

      std::vector<uint8_t> write_value{byte};
      scanner_->writeAdr(write_value, row_abs_address + res);

      read_region_.read_bytes[row * config_.bytes_per_row + res] = byte;
    }

    std::string print_buf;
    for (int32_t k = 0; k < config_.bytes_per_row; ++k) {
      int32_t hex_index = static_cast<int32_t>(row) * config_.bytes_per_row + k;
      if (std::isprint(static_cast<uint8_t>(read_region_.read_bytes[static_cast<uint64_t>(hex_index)])))
        print_buf += static_cast<char>(read_region_.read_bytes[static_cast<uint64_t>(hex_index)]);
      else
        print_buf += '.';
    }
    ImGui::TextUnformatted(print_buf.c_str());
  }

  if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) editing_index_ = -1;

  ImGui::EndTable();
  ImGui::EndChild();
}

// returns the hex_index to be edited. -1 if no edit.
int32_t HexW::drawRow(const std::vector<uint8_t>& bytes, int32_t hex_index) {
  int32_t edit_index = -1;
  for (uint16_t i = 0; i < config_.bytes_per_row; ++i) {
    ImGui::PushID(hex_index);

    if (editing_index_ == hex_index) {
      ImGui::SetNextItemWidth(25.0F);
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
      if (ImGui::InputText(
              "##edit", &edit_str_buf, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue))
        edit_index = hex_index;
      ImGui::PopStyleVar();
    } else {
      char label[4];
      snprintf(label, sizeof(label), "%02X", bytes[hex_index]);
      if (ImGui::Selectable(label, false, ImGuiSelectableFlags_AllowDoubleClick)) {
        edit_str_buf = hexToStr(bytes[hex_index]);
        editing_index_ = hex_index;
      }
    }

    ++hex_index;
    ImGui::PopID();
    ImGui::TableNextColumn();
  }
  return edit_index;
}

void HexW::drawConfigPopup() {
  if (!ImGui::BeginPopupModal("Config", nullptr)) return;

  ImGui::InputScalar("Bytes before", ImGuiDataType_U16, &config_.bytes_before);
  ImGui::InputScalar("Bytes after", ImGuiDataType_U16, &config_.bytes_after);
  ImGui::InputScalar("Bytes per row", ImGuiDataType_U16, &config_.bytes_per_row);

  if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();

  ImGui::EndPopup();
}
