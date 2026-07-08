#pragma once

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "types.h"

GLFWwindow* initaliseImgui(const std::string &imgui_init_path_str);
void exitImgui(GLFWwindow* window);
void start_frame();
void endFrame(int display_w, int display_h, ImVec4 clear_color, GLFWwindow* window);
void setDefaultDisplay();

std::string mainMenuBarCycle(bool& target_popup_clicked, bool& map_popup_clicked, bool& log_w_enabled, bool& hex_w_enabled, bool& data_inspector_w_enabled, bool& pointer_w_enabled);
bool getTargetValue(TargetType TargetType, std::vector<uint8_t>& write_to, ImGuiInputTextFlags flags = 0);

void printData(const std::vector<uint8_t>& data, TargetType target_type);
