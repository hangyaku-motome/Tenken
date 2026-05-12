#pragma once

#include "TargetPopUp.hpp"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
GLFWwindow *initalise_main();
void exit_main(GLFWwindow *window);
void start_frame();
void end_frame(int display_w, int display_h, ImVec4 clear_color,
               GLFWwindow *window);
void SetDefaultDisplay();
void MainMenuBarCycle(TargetPopUp &TargetPUp);
std::string ValToStr(const std::vector<uint8_t> &Bytes, TargetTypeT TargetType);
std::string RelativeStatusToStr(RelativeStatus Status);
bool GetTargetValue(TargetTypeT TargetType, std::vector<uint8_t> &write_to,
                    ImGuiInputTextFlags flags = 0);
std::string TargetTypeToStr(TargetTypeT TargetType);
template <typename T> T readAs(const std::vector<uint8_t> &buffer);
