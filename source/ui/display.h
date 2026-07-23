#pragma once

#include <imgui.h>
#include <imgui-filebrowser/imfilebrowser.h>

#include "GLFW/glfw3.h"
#include "types.h"

GLFWwindow* initaliseImgui(const std::string& imgui_init_path_str);
void exitImgui(GLFWwindow* window);
void start_frame();
void endFrame(int display_w, int display_h, ImVec4 clear_color, GLFWwindow* window);
void setDefaultDisplay();

std::string mainMenuBarCycle(ImGui::FileBrowser& save_dialog,
                             ImGui::FileBrowser& load_dialog,
                             bool& target_popup_clicked,
                             bool& map_popup_clicked,
                             bool& log_w_enabled,
                             bool& hex_w_enabled,
                             bool& data_inspector_w_enabled,
                             bool& pointer_w_enabled);
bool getTargetValue(TargetType target_type, std::vector<uint8_t>& write_to, ImGuiInputTextFlags flags = 0);

void printData(const std::vector<uint8_t>& data, TargetType target_type);
