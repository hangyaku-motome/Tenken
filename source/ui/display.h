#pragma once

#include "MapPopUp.h"
#include "TargetPopUp.h"
#include "imgui.h"
#include "types.h"
#include "utils.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

GLFWwindow *initalise_main();
void exit_main(GLFWwindow *window);
void start_frame();
void end_frame(int display_w, int display_h, ImVec4 clear_color, GLFWwindow *window);
void SetDefaultDisplay();
void MainMenuBarCycle(TargetPopUp &TargetPUp, MapsPopUp &MapPup);
bool GetTargetValue(TargetTypeT TargetType, std::vector<uint8_t> &write_to,
                    ImGuiInputTextFlags flags = 0);

void printData(const std::vector<uint8_t> &data, TargetTypeT TargetType);