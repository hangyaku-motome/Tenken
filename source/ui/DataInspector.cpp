#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <string>

#include "DataInspectorW.h"
#include "types.h"
#include "utils.h"

// TODO: readAround exists both in here and in HexW. Make a common function in utils or somehwere.

bool DataInspectorW::InitW() { return ImGui::Begin("Inspector"); }

void DataInspectorW::EndW() { ImGui::End(); }

void DataInspectorW::CycleW() {
  if (!enabled_) return;
  if (!InitW()) {
    EndW();
    return;
  }

  ImGui::InputScalar("Go to:", ImGuiDataType_U64, &addressbuffer_, nullptr, nullptr, "%016lx");
  ImGui::SameLine();
  if (ImGui::Button("Go")) {
    currentaddress_ = addressbuffer_;
    bytes_ = readAround(currentaddress_);
  }
  if (ImGui::Button("Refresh")) {
    bytes_ = readAround(currentaddress_);
  }
  if (ImGui::Button("Config")) popupclicked_ = true;

  TypePopUp();

  if (!bytes_.empty()) RenderTable();

  EndW();
}

void DataInspectorW::RenderTable() {
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 50.0F, 150.0F);
  if (!ImGui::BeginChild("inspectortable", {0, avail - context_height})) return;

  int enabled_count = types_.doubleT + types_.u8 + types_.u16 + types_.u32 + types_.u64 + types_.s8 + types_.s16 +
                      types_.s32 + types_.s64 + types_.floatT + types_.stringT + 1;  // 1 for offset.

  if (!ImGui::BeginTable("Inspect View", enabled_count, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
    ImGui::EndChild();
    return;
  }

  int32_t offset = -BYTES_BEFORE;

  ImGui::TableSetupColumn("Offset");
  if (types_.u8) ImGui::TableSetupColumn("u8");
  if (types_.u16) ImGui::TableSetupColumn("u16");
  if (types_.u32) ImGui::TableSetupColumn("u32");
  if (types_.u64) ImGui::TableSetupColumn("u64");
  if (types_.s8) ImGui::TableSetupColumn("s8");
  if (types_.s16) ImGui::TableSetupColumn("s16");
  if (types_.s32) ImGui::TableSetupColumn("s32");
  if (types_.s64) ImGui::TableSetupColumn("s64");
  if (types_.floatT) ImGui::TableSetupColumn("float");
  if (types_.doubleT) ImGui::TableSetupColumn("double");
  if (types_.stringT) ImGui::TableSetupColumn("string");
  ImGui::TableHeadersRow();

  // there is a much shorter way to do this but I will deal with that later.
  for (uint32_t row = 0; row + LIMIT < bytes_.size(); ++row) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(std::to_string(offset).c_str());

    if (types_.u8) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(
          dataToStr<uint8_t>(std::vector(bytes_.begin() + row, bytes_.begin() + row + sizeof(uint8_t))).c_str());
    }
    if (types_.u16) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(
          dataToStr<uint16_t>(std::vector(bytes_.begin() + row, bytes_.begin() + row + sizeof(uint16_t))).c_str());
    }
    if (types_.u32) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(
          dataToStr<uint32_t>(std::vector(bytes_.begin() + row, bytes_.begin() + row + sizeof(uint32_t))).c_str());
    }
    if (types_.u64) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(
          dataToStr<uint64_t>(std::vector(bytes_.begin() + row, bytes_.begin() + row + sizeof(uint64_t))).c_str());
    }
    if (types_.s8) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(
          dataToStr<int8_t>(std::vector(bytes_.begin() + row, bytes_.begin() + row + sizeof(int8_t))).c_str());
    }
    if (types_.s16) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(
          dataToStr<int16_t>(std::vector(bytes_.begin() + row, bytes_.begin() + row + sizeof(int16_t))).c_str());
    }
    if (types_.s32) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(
          dataToStr<int32_t>(std::vector(bytes_.begin() + row, bytes_.begin() + row + sizeof(int32_t))).c_str());
    }
    if (types_.s64) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(
          dataToStr<int64_t>(std::vector(bytes_.begin() + row, bytes_.begin() + row + sizeof(int64_t))).c_str());
    }
    if (types_.floatT) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(
          dataToStr<float>(std::vector(bytes_.begin() + row, bytes_.begin() + row + sizeof(float))).c_str());
    }
    if (types_.doubleT) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(
          dataToStr<double>(std::vector(bytes_.begin() + row, bytes_.begin() + row + sizeof(double))).c_str());
    }
    if (types_.stringT) {
      std::string printbuf;
      for (int32_t i = 0; i < LIMIT; ++i) {
        if (std::isprint(static_cast<uint8_t>(bytes_[row + i])))
          printbuf += bytes_[row + i];
        else
          printbuf += '.';
      }
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(printbuf.c_str());
    }


    ++offset;
  }
  ImGui::EndTable();
  ImGui::EndChild();
}

std::vector<uint8_t> DataInspectorW::readAround(const uint64_t adr) {
  uint64_t search_start = adr - BYTES_BEFORE < 0 ? 0 : adr - BYTES_BEFORE;
  return scanner_->readAdr(search_start, BYTES_AFTER + BYTES_BEFORE);
}

void DataInspectorW::TypePopUp() {
  if (popupclicked_) {
    ImGui::OpenPopup("Target List");
    popupclicked_ = false;
  }
  if (!ImGui::BeginPopupModal("Target List", nullptr, popup_flags)) return;

  ImGui::Checkbox("unsigned 1 byte", &types_.u8);
  ImGui::Checkbox("unsigned 2 byte", &types_.u16);
  ImGui::Checkbox("unsigned 4 byte", &types_.u32);
  ImGui::Checkbox("unsigned 8 byte", &types_.u64);
  ImGui::Checkbox("signed 1 byte", &types_.s8);
  ImGui::Checkbox("signed 2 byte", &types_.s16);
  ImGui::Checkbox("signed 4 byte", &types_.s32);
  ImGui::Checkbox("signed 8 byte", &types_.s64);
  ImGui::Checkbox("float", &types_.floatT);
  ImGui::Checkbox("double", &types_.doubleT);
  ImGui::Checkbox("string", &types_.stringT);

  if (ImGui::Button("Cancel")) {
    ImGui::CloseCurrentPopup();
  }
  ImGui::EndPopup();
}
