#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <string>

#include "DataInspectorW.h"
#include "utils.h"

static constexpr auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;

bool DataInspectorW::InitW() { return ImGui::Begin("Inspector"); }

void DataInspectorW::EndW() { ImGui::End(); }

void DataInspectorW::CycleW() {
  if (!enabled_) return;
  if (!InitW()) return;

  ImGui::InputScalar("Go to:", ImGuiDataType_U64, &currentaddress_, nullptr, nullptr, "%016lx");

  if (ImGui::Button("Go")) {
    currentaddress_ = addressbuffer_;
    readAround(currentaddress_);
  }
  if (ImGui::Button("Refresh")) {
    readAround(currentaddress_);
  }
  if (ImGui::Button("Config")) TypePopUp();

  EndW();
}

void DataInspectorW::RenderTable() {
  float avail = ImGui::GetContentRegionAvail().y;
  float context_height = std::clamp(avail * 0.1F, 50.0F, 150.0F);
  if (!ImGui::BeginChild("inspectortable", {0, avail - context_height})) return;

  if (!ImGui::BeginTable("Inspect View", 11, ImGuiTableFlags_ScrollY)) {
    ImGui::EndChild();
    return;
  }

  int32_t offset = -BYTES_BEFORE;

  for (uint32_t row = 0; row + LIMIT < bytes_.size(); ++row) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(std::to_string(offset).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<uint8_t>(std::vector(bytes_.begin(), bytes_.begin() + sizeof(uint8_t))).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<uint16_t>(std::vector(bytes_.begin(), bytes_.begin() + sizeof(uint16_t))).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<uint32_t>(std::vector(bytes_.begin(), bytes_.begin() + sizeof(uint32_t))).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<uint64_t>(std::vector(bytes_.begin(), bytes_.begin() + sizeof(uint64_t))).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<int8_t>(std::vector(bytes_.begin(), bytes_.begin() + sizeof(int8_t))).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<int16_t>(std::vector(bytes_.begin(), bytes_.begin() + sizeof(int16_t))).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<int32_t>(std::vector(bytes_.begin(), bytes_.begin() + sizeof(int32_t))).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<int64_t>(std::vector(bytes_.begin(), bytes_.begin() + sizeof(int64_t))).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<float>(std::vector(bytes_.begin(), bytes_.begin() + sizeof(float))).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<double>(std::vector(bytes_.begin(), bytes_.begin() + sizeof(double))).c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(dataToStr<std::string>(std::vector(bytes_.begin(), bytes_.begin() + LIMIT)).c_str());
  }
}

std::vector<uint8_t> DataInspectorW::readAround(const uint64_t adr) {
  uint64_t search_start = adr - BYTES_BEFORE < 0 ? 0 : adr - BYTES_BEFORE;
  return scanner_->readAdr(search_start, BYTES_AFTER + BYTES_BEFORE);
}

void DataInspectorW::TypePopUp() {
  if (!ImGui::BeginPopupModal("Target List", nullptr, flags)) return;
  ImGui::Checkbox("unsigned 1 byte", &types_.s8);
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
  ImGui::EndPopup();
}
