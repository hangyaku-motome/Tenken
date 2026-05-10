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
std::string BytesToHex(const std::vector<uint8_t> &Data);
std::string ValToStr(const std::vector<uint8_t> &Bytes,
                     const TargetTypeT TargetType);
std::string RelativeStatusToStr(const RelativeStatus Status);
bool GetTargetValue(const TargetTypeT TargetType,
                    std::vector<uint8_t> &write_to,
                    ImGuiInputTextFlags flags = 0);
template <typename T> T readAs(const std::vector<uint8_t> &buffer);