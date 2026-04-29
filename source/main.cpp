#include "HitsW.h"
#include "LogW.h"
#include "SearchW.h"
#include "TargetPopUp.hpp"
#include "display.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

// some variables and types have the same name. fix that. It's confusing.
// Also I just noticed there is an input delay...That compounds to a lot. I
// think we'll need to implement multithreading..soon.

void SetDisplayInfo(GLFWwindow *window, DisplayInfoT &DisplayInfo);

int main() {

  GLFWwindow *window = initalise_main();

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  ImGuiWindowFlags flagsWindowDefault = ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoCollapse;

  ActiveInfoT ActiveInfo;
  DisplayInfoT DisplayInfo;

  LogW LogObj;
  SearchW SearchObj;
  HitsW HitObj;
  TargetPopUp TargetPUp;

  // Main loop.
  while (!glfwWindowShouldClose(window)) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    LogEventsT LogEvents;

    start_frame();

    SetDisplayInfo(window, DisplayInfo);

    MainMenuBarCycle(TargetPUp);

    // target popup.
    TargetPUp.CyclePUp(LogEvents, ActiveInfo);

    // Hits.
    HitObj.CycleW(DisplayInfo.Hit, flagsWindowDefault);

    // Search.
    SearchObj.CycleW(DisplayInfo.Search, flagsWindowDefault);

    // Log.
    LogObj.CycleW(DisplayInfo.Log, flagsWindowDefault, LogEvents);

    end_frame(DisplayInfo.display_w, DisplayInfo.display_h, clear_color,
              window);
  }

  // Clean up.
  exit_main(window);
}
