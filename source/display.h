#pragma once

#include "HitsW.h"
#include "LogW.h"
#include "SearchW.h"
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
void SetDisplayInfo(GLFWwindow *window, DisplayInfoT DisplayInfo,
                    ImGuiWindowFlags flagsWindowDefault, LogW &LogObj,
                    SearchW &SearchObj, HitsW &HitObj);
void MainMenuBarCycle(TargetPopUp &TargetPUp);
std::string BytesToHex(const std::vector<uint8_t> &Data);