#pragma once

#include <imgui-filebrowser/imfilebrowser.h>
#include <imgui.h>

#include "GLFW/glfw3.h"
#include "types.h"

GLFWwindow* initaliseImgui(const std::string& imgui_init_path_str);
void exitImgui(GLFWwindow* window);
void start_frame();
void endFrame(int display_w, int display_h, ImVec4 clear_color, GLFWwindow* window);
void setDefaultDisplay();

// I Coulddd put this to another file put...eh, TODO: later.


bool getTargetValue(TargetType target_type, std::vector<uint8_t>& write_to, ImGuiInputTextFlags flags = 0);

void printData(const std::vector<uint8_t>& data, TargetType target_type);
