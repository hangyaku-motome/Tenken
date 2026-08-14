#include "DataInspectorW.h"

#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <string>

#include "types.h"
#include "utils.h"

// readAround exists both in here and in HexW. Make a common function in utils or somehwere...Or don't. Not a big deal
// for now.

bool DataInspectorW::initW() { return ImGui::Begin("Inspector"); }

void DataInspectorW::endW() { ImGui::End(); }

void DataInspectorW::cycleW() {
  if (!enabled_) return;
  if (!initW()) {
    endW();
    return;
  }
  if (not scanner_->isAttached()) {
    ImGui::Text("No target!");
    endW();
    return;
  }

  ImGui::InputScalar("Go to:", ImGuiDataType_U64, &address_buf, nullptr, nullptr, "%016lx");
  ImGui::SameLine();
  if (ImGui::Button("Go")) {
    read_region_ = scanner_->readAround(address_buf, bytes_before, bytes_after);
    current_address_ = address_buf;
  }
  if (ImGui::Button("Refresh")) {
    read_region_ = scanner_->readAround(address_buf, bytes_before, bytes_after);
    current_address_ = address_buf;
  }
  if (ImGui::Button("Config")) popup_clicked_ = true;

  configPopup();

  if (!read_region_.read_bytes.empty()) renderTable();

  endW();
}

void DataInspectorW::renderTable() {
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 50.0F, 150.0F);
  if (!ImGui::BeginChild("inspectortable", {0, avail - context_height})) {
    ImGui::EndChild();
    return;
  }

  int enabled_count = types_.f64 + types_.u8 + types_.u16 + types_.u32 + types_.u64 + types_.s8 + types_.s16 +
                      types_.s32 + types_.s64 + types_.f32 + types_.string + types_.ptr + 1;  // 1 for offset.

  if (!ImGui::BeginTable("Inspect View", enabled_count, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
    ImGui::EndChild();
    return;
  }

  int32_t offset = read_region_.offset_from_adr;

  ImGui::TableSetupColumn("Offset");
  if (types_.u8) ImGui::TableSetupColumn("u8");
  if (types_.u16) ImGui::TableSetupColumn("u16");
  if (types_.u32) ImGui::TableSetupColumn("u32");
  if (types_.u64) ImGui::TableSetupColumn("u64");
  if (types_.s8) ImGui::TableSetupColumn("s8");
  if (types_.s16) ImGui::TableSetupColumn("s16");
  if (types_.s32) ImGui::TableSetupColumn("s32");
  if (types_.s64) ImGui::TableSetupColumn("s64");
  if (types_.f32) ImGui::TableSetupColumn("float");
  if (types_.f64) ImGui::TableSetupColumn("double");
  if (types_.string) ImGui::TableSetupColumn("string");
  if (types_.ptr) ImGui::TableSetupColumn("hex");
  ImGui::TableHeadersRow();

  // there is a much shorter way to do this but I will deal with that later. wayy later.
  for (uint32_t row = 0; row + limit_ < read_region_.read_bytes.size(); ++row) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(std::to_string(offset).c_str());

    if (types_.u8) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(dataToStr<uint8_t>(std::vector(read_region_.read_bytes.begin() + row,
                                                            read_region_.read_bytes.begin() + row + sizeof(uint8_t)))
                                 .c_str());
    }
    if (types_.u16) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(dataToStr<uint16_t>(std::vector(read_region_.read_bytes.begin() + row,
                                                             read_region_.read_bytes.begin() + row + sizeof(uint16_t)))
                                 .c_str());
    }
    if (types_.u32) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(dataToStr<uint32_t>(std::vector(read_region_.read_bytes.begin() + row,
                                                             read_region_.read_bytes.begin() + row + sizeof(uint32_t)))
                                 .c_str());
    }
    if (types_.u64) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(dataToStr<uint64_t>(std::vector(read_region_.read_bytes.begin() + row,
                                                             read_region_.read_bytes.begin() + row + sizeof(uint64_t)))
                                 .c_str());
    }
    if (types_.s8) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(dataToStr<int8_t>(std::vector(read_region_.read_bytes.begin() + row,
                                                           read_region_.read_bytes.begin() + row + sizeof(int8_t)))
                                 .c_str());
    }
    if (types_.s16) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(dataToStr<int16_t>(std::vector(read_region_.read_bytes.begin() + row,
                                                            read_region_.read_bytes.begin() + row + sizeof(int16_t)))
                                 .c_str());
    }
    if (types_.s32) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(dataToStr<int32_t>(std::vector(read_region_.read_bytes.begin() + row,
                                                            read_region_.read_bytes.begin() + row + sizeof(int32_t)))
                                 .c_str());
    }
    if (types_.s64) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(dataToStr<int64_t>(std::vector(read_region_.read_bytes.begin() + row,
                                                            read_region_.read_bytes.begin() + row + sizeof(int64_t)))
                                 .c_str());
    }
    if (types_.f32) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(dataToStr<float>(std::vector(read_region_.read_bytes.begin() + row,
                                                          read_region_.read_bytes.begin() + row + sizeof(float)))
                                 .c_str());
    }
    if (types_.f64) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(dataToStr<double>(std::vector(read_region_.read_bytes.begin() + row,
                                                           read_region_.read_bytes.begin() + row + sizeof(double)))
                                 .c_str());
    }
    if (types_.string) {
      std::string printbuf;
      for (int32_t i = 0; i < limit_; ++i) {
        if (std::isprint(static_cast<uint8_t>(read_region_.read_bytes[row + static_cast<uint32_t>(i)])))
          printbuf += static_cast<char>(read_region_.read_bytes[row + static_cast<uint32_t>(i)]);
        else
          printbuf += '.';
      }
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(printbuf.c_str());
    }
    if (types_.ptr) {
      ImGui::TableNextColumn();
      ImGui::Text("%lx",
                  dataToType<uint64_t>(std::vector(read_region_.read_bytes.begin() + row,
                                                   read_region_.read_bytes.begin() + row + sizeof(double))));
    }

    ++offset;
  }
  ImGui::EndTable();
  ImGui::EndChild();
}

void DataInspectorW::configPopup() {
  if (popup_clicked_) {
    ImGui::OpenPopup("Target List");
    popup_clicked_ = false;
  }
  if (!ImGui::BeginPopupModal("Target List", nullptr, DefaultPopupFlags)) return;

  ImGui::Checkbox("unsigned 1 byte", &types_.u8);
  ImGui::Checkbox("unsigned 2 byte", &types_.u16);
  ImGui::Checkbox("unsigned 4 byte", &types_.u32);
  ImGui::Checkbox("unsigned 8 byte", &types_.u64);
  ImGui::Checkbox("signed 1 byte", &types_.s8);
  ImGui::Checkbox("signed 2 byte", &types_.s16);
  ImGui::Checkbox("signed 4 byte", &types_.s32);
  ImGui::Checkbox("signed 8 byte", &types_.s64);
  ImGui::Checkbox("float", &types_.f32);
  ImGui::Checkbox("double", &types_.f64);
  ImGui::Checkbox("string", &types_.string);
  ImGui::Checkbox("ptr", &types_.ptr);

  if (ImGui::Button("Cancel")) {
    ImGui::CloseCurrentPopup();
  }
  ImGui::EndPopup();
}
