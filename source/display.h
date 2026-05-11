#pragma once

#include "LogW.h"
#include "TargetPopUp.hpp"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <optional>

GLFWwindow *initalise_main();
void exit_main(GLFWwindow *window);
void start_frame();
void end_frame(int display_w, int display_h, ImVec4 clear_color,
               GLFWwindow *window);
void SetDefaultDisplay();
void MainMenuBarCycle(TargetPopUp &TargetPUp);
std::string BytesToHex(const std::vector<uint8_t> &Data);
std::string ValToStr(const std::vector<uint8_t> &Bytes,
                     const TargetTypeT TargetType);
std::string RelativeStatusToStr(const RelativeStatus Status);
bool GetTargetValue(const TargetTypeT TargetType,
                    std::vector<uint8_t> &write_to,
                    ImGuiInputTextFlags flags = 0);
std::string TargetTypetoStr(const TargetTypeT TargetType);
template <typename T> T readAs(const std::vector<uint8_t> &buffer);

class ContextDisplay {
  float button_w = 150.0f;
  float button_h = 150.0f;
  float slider_w = 150.0f;
  float checkbox_w = 50.0f;
  bool IsRefresh = false;

  float DrawRefreshInterval(const float RefreshDuration);
  bool DrawRefreshAllButton();
  bool DrawRefreshContextButton();
  void AlignButtons();
  template <typename T> void DrawContextMenu(const T Entry) {
    int constexpr BYTES_BEFORE = 32;
    int constexpr BYTES_AFTER = 32;
    int constexpr BYTES_PER_ROW = 16;

    if (Entry.bytes_around.size() !=
        Entry.value.size() + BYTES_BEFORE + BYTES_AFTER) {
      Log::Error("Entry " + std::to_string(Entry.location) +
                 " is near a memory region and I have not implemented a way to "
                 "reliably display bytes for that case. TODO later.");
      return;
    }

    for (int i = 0; i < Entry.bytes_around.size(); ++i) {
      ImGui::SameLine(0, 4);
      if (i % 32 == 0)
        ImGui::NewLine();
      else if (i % 8 == 0) {
        ImGui::Text(" ");
        ImGui::SameLine(0, 4);
      }
      // logic might be wrong let's see
      if (i >= BYTES_BEFORE && i + BYTES_AFTER < Entry.bytes_around.size()) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 160, 100, 255));
        ImGui::Text("%02X", Entry.bytes_around[i]);
        ImGui::PopStyleColor();
      } else
        ImGui::Text("%02X", Entry.bytes_around[i]);
    }
  }

public:
  template <typename T>
  Action CycleContext(const uint64_t selected_row, const T Entry,
                      const float RefreshDuration = 0,
                      std::optional<bool> SetIsRefresh = std::nullopt) {
    bool disable_context_refresh = true;
    if (SetIsRefresh.has_value())
      IsRefresh = SetIsRefresh.value();
    printf("Isrefresh %i\n", IsRefresh);

    // if favourite also set regular refresh value and duration. unless we
    // wanna regularly refresh all favourites at the same time.
    DrawContextMenu(Entry);
    disable_context_refresh = false;

    AlignButtons();
    if (disable_context_refresh)
      ImGui::BeginDisabled();
    auto RefreshContext = DrawRefreshContextButton();
    if (disable_context_refresh)
      ImGui::EndDisabled();
    ImGui::SameLine();
    float NewRefreshDuration = DrawRefreshInterval(RefreshDuration);
    ImGui::SameLine();
    auto RefreshAll = DrawRefreshAllButton();

    if (RefreshContext)
      return Action{OpType::REFRESH, DataType::INVALID, selected_row};
    if (RefreshAll)
      return Action{OpType::REFRESH_ALL, DataType::INVALID, selected_row};

    if (NewRefreshDuration == -2)
      return {};
    printf("constructing action.\n");
    Action ReturnAction;
    ReturnAction.Type = OpType::REGULAR_REFRESH;
    ReturnAction.WorkOn = DataType::INVALID;
    ReturnAction.seconds = NewRefreshDuration;
    ReturnAction.index = selected_row;
    return ReturnAction;

    return Action{OpType::NONE};
  }
};
