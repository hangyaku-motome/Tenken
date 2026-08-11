#include "MainMenuBar.h"

#include <imgui.h>

#include "Platform.h"
#include "types.h"
#include "utils.h"

// if we had put Display() before Open...dialog and pop up will pop up NEXT frame, not current one where it's clicked.
// but then also they would be displayed even if not begin main menu bar..but IDK if that could happen. whether or not
// that's acceptable, I've yet to decide on. I'm sure stuff like this happens on other places in the codebase with other
// UI elements; with some I remember.
PendingAction MainMenuBar::cycle(const std::string& target_name) {
  if (!ImGui::BeginMainMenuBar()) return {};

  cycleFileMenu();

  save_dialog_.Display();
  load_dialog_.Display();

  PendingAction action;

  action = cyclePointerPopup(target_name);
  ptr_dialog_.Display();

  if (ptr_dialog_.HasSelected()) {
    action = Action::ResolvePointerResult{.save_path = ptr_dialog_.GetSelected(), .target_address = adr_buf_};
    ptr_dialog_.Close();
    ImGui::CloseCurrentPopup();
  }
  if (save_dialog_.HasSelected()) {
    action = Action::SaveTenken{.path = save_dialog_.GetSelected()};
    save_dialog_.Close();
  }
  if (load_dialog_.HasSelected()) {
    action = Action::LoadTenken{.path = load_dialog_.GetSelected()};
    load_dialog_.Close();
  }

  cycleUtilsMenu();

  ImGui::EndMainMenuBar();

  return action;
}

void MainMenuBar::cycleFileMenu() {
  if (!ImGui::BeginMenu("File")) return;
  if (ImGui::MenuItem("New Target")) windows_.target_popup = true;

  if (ImGui::MenuItem("Save")) save_dialog_.Open();
  if (ImGui::MenuItem("Load")) load_dialog_.Open();
  if (ImGui::MenuItem("Resolve Pointers")) ptr_resolve_popup = true;

  ImGui::EndMenu();
}

void MainMenuBar::cycleUtilsMenu() {
  if (!ImGui::BeginMenu("Utils")) return;
  if (ImGui::MenuItem("View Regions")) windows_.map_popup = true;

  if (ImGui::MenuItem("Toggle Log window.", nullptr, windows_.log_w, true)) windows_.log_w = !windows_.log_w;

  if (ImGui::MenuItem("Toggle Hex window.", nullptr, windows_.hex_w, true)) windows_.hex_w = !windows_.hex_w;

  if (ImGui::MenuItem("Toggle Data Inspector window.", nullptr, windows_.data_inspector, true))
    windows_.data_inspector = !windows_.data_inspector;
  if (ImGui::MenuItem("Toggle Pointer window.", nullptr, windows_.pointer_w, true))
    windows_.pointer_w = !windows_.pointer_w;

  ImGui::EndMenu();
}

PendingAction MainMenuBar::cyclePointerPopup(const std::string& exec_name) {
  if (ptr_resolve_popup) {
    ImGui::OpenPopup("Pointer Resolving");
    ptr_resolve_popup = false;
  }

  if (!ImGui::BeginPopupModal("Pointer Resolving", nullptr, PopupFlags)) return {};

  if (exec_name.empty()) {
    ImGui::Text(
        "You either didn't actually choose a process yet...OR something is very much wrong here. a bug, if you will.");
    if (ImGui::Button("Run away")) {
      ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
      return {};
    }
    ImGui::EndPopup();
    return {};
  }

  ImGui::Text("First, input the new address of the target in the process.");
  ImGui::InputScalar("##", ImGuiDataType_U64, &adr_buf_, nullptr, nullptr, "%016lx");

  ImGui::Text("Click latest to automatically get the latest done pointer scan/filtering, otherwise manually choose "
              "the file yourself");

  ImGui::BeginDisabled(adr_buf_ == 0);

  PendingAction action;

  if (ImGui::Button("Latest")) {
    action = Action::ResolvePointerResult{
        .save_path = getLatestFile(Platform::getTenkenStatePath() / "Pointer" / exec_name), .target_address = adr_buf_};
    ImGui::CloseCurrentPopup();
  }

  if (ImGui::Button("Lemme choose")) ptr_dialog_.Open();

  ImGui::EndDisabled();

  if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();

  ImGui::EndPopup();

  return action;
}
