#include "HitsW.h"
#include "LogW.h"
#include "Scanner.h"
#include "SearchW.h"
#include "TargetPopUp.hpp"
#include "display.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
void SetDisplayInfo(GLFWwindow *window, DisplayInfoT &DisplayInfo);

int main() {

  // Start up Dear ImGui.
  GLFWwindow *window = initalise_main();
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  ImGuiWindowFlags flagsWindowDefault = ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoCollapse;

  ChosenParams ActiveInfo;
  DisplayInfoT DisplayInfo;

  LogW LogObj;
  SearchW SearchObj;
  HitsW HitObj;
  TargetPopUp TargetPUp;
  Scanner ScannerObj;

  // Main loop.
  while (!glfwWindowShouldClose(window)) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    start_frame();

    SetDisplayInfo(window, DisplayInfo);

    MainMenuBarCycle(TargetPUp);

    // target popup.
    if (TargetPUp.CyclePUp(ActiveInfo)) {
      SearchObj.ClearWindow();
      ActiveInfo.TargetValInfo.TargetType = TargetTypeT::Invalid;
      ActiveInfo.TargetValInfo.IsUnsigned = 0;
      ActiveInfo.TargetValInfo.value.clear();
    }

    // Hits.
    HitObj.CycleW(DisplayInfo.Hit, flagsWindowDefault, ScannerObj.Hits);

    // Search.
    if (SearchObj.CycleW(DisplayInfo.Search, flagsWindowDefault, ActiveInfo) ==
        1) {
      Log::Info("Starting initial scan...");
      ScannerObj.Start(ActiveInfo.TargetProc.pid);
      ScannerObj.StartScan(ActiveInfo.TargetValInfo);
    }

    // Log.
    LogObj.CycleW(DisplayInfo.Log, flagsWindowDefault);

    end_frame(DisplayInfo.display_w, DisplayInfo.display_h, clear_color,
              window);
  }

  exit_main(window);
}
