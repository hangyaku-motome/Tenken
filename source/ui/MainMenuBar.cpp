#include "MainMenuBar.h"

#include <imgui.h>

#include "types.h"
#include "utils.h"

PendingAction MainMenuBar::cycle(const std::string& exec_name) {
  PendingAction action;
  save_dialog.Display();
  load_dialog.Display();
  ptr_dialog.Display();

  if (save_dialog.HasSelected()) {
    action = Action::SaveTenken{.path = save_dialog.GetSelected()};
    save_dialog.Close();
  }

  if (load_dialog.HasSelected()) {
    action = Action::LoadTenken{.path = load_dialog.GetSelected()};
    load_dialog.Close();
  }

  if (ptr_dialog.HasSelected()) {
    action = Action::ResolvePointerResult{.exec_name = exec_name, .target_address = adr_buf};
    load_dialog.Close();
  }

  if (!ImGui::BeginMainMenuBar()) return action;

  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("New Target")) windows.target_popup = true;

    if (ImGui::MenuItem("Save")) save_dialog.Open();
    if (ImGui::MenuItem("Load")) load_dialog.Open();
    if (ImGui::MenuItem("Pointer Resolving")) ImGui::OpenPopup("Target Address");
  }

  ImGui::EndMenu();

  if (ImGui::BeginPopupModal("Pointer Resolving", nullptr, PopupFlags)) {
    ImGui::Text("First, input the new address of the target in the process.");
    ImGui::InputScalar("##", ImGuiDataType_U64, &adr_buf);

    ImGui::Text("Click latest to automatically get the latest done pointer scan/filtering, otherwise manually choose "
                "the file yourself");

    ImGui::BeginDisabled(adr_buf == 0);

    if (ImGui::Button("Latest"))
      action = Action::ResolvePointerResult{.exec_name = getLatestScan(exec_name), .target_address = adr_buf};

    if (ImGui::Button("Lemme choose")) ptr_dialog.Open();

    ImGui::EndDisabled();
  }

  if (ImGui::BeginMenu("Utils")) {
    if (ImGui::MenuItem("View Regions")) windows.map_popup = true;

    if (ImGui::MenuItem("Toggle Log window.", nullptr, windows.log_w, true)) windows.log_w = !windows.log_w;

    if (ImGui::MenuItem("Toggle Hex window.", nullptr, windows.hex_w, true)) windows.hex_w = !windows.hex_w;

    if (ImGui::MenuItem("Toggle Data Inspector window.", nullptr, windows.data_inspector, true))
      windows.data_inspector = !windows.data_inspector;
    if (ImGui::MenuItem("Toggle Pointer window.", nullptr, windows.pointer_w, true))
      windows.pointer_w = !windows.pointer_w;

    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();

  return action;
}
