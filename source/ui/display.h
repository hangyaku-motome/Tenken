#pragma once

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>

#include "imgui.h"
#include "types.h"

GLFWwindow* initalise_main(const std::filesystem::path &ImGuiInitPath);
void exit_main(GLFWwindow* window);
void start_frame();
void end_frame(int display_w, int display_h, ImVec4 clear_color, GLFWwindow* window);
void SetDefaultDisplay();

std::string MainMenuBarCycle(bool& TargetPUpClicked, bool& MapPupClicked, bool& LogWEnabled, bool& HexWEnabled, bool& DataInspectorWEnabled);
bool GetTargetValue(TargetTypeT TargetType, std::vector<uint8_t>& write_to, ImGuiInputTextFlags flags = 0);

void printData(const std::vector<uint8_t>& data, TargetTypeT TargetType);
