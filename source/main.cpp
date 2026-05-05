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
#include <cstdint>
void SetDisplayInfo(GLFWwindow *window, DisplayInfoT &DisplayInfo);

// Put window dimensions in objects. And have a single function called at the
// start that recalculates it all *if* dimensions change. Also we should NOT try
// to put the calculation inside the objects. Sounds dumb. Pass value only.

// A function that refreshes Hit values and bytes regularly in sccanner?
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

    SetDisplayInfo(window, DisplayInfo, flagsWindowDefault, LogObj, SearchObj,
                   HitObj);

    MainMenuBarCycle(TargetPUp);

    // target popup.
    if (TargetPUp.CyclePUp(ActiveInfo) == "new target") {
      SearchObj = {};
      HitObj = {};
      ActiveInfo.TargetValInfo = {};
      ScannerObj = {};
    }

    // Hits.
    auto RefreshType = HitObj.CycleW(ScannerObj.Hits, ActiveInfo.TargetValInfo);

    if (RefreshType == "refresh all") {
      for (uint64_t i = 0; i < ScannerObj.Hits.size(); ++i)
        ScannerObj.RescanHit(i);
    } else if (RefreshType == "refresh context") {
      ScannerObj.RescanHit(HitObj.selected_row);
    }

    // Search.
    if (SearchObj.CycleW(ActiveInfo) == 1) {
      Log::Info("Starting initial scan...");
      ScannerObj.Start(ActiveInfo.TargetProc.pid);
      ScannerObj.StartScan(ActiveInfo.TargetValInfo);
    }

    // Log.
    LogObj.CycleW();

    end_frame(DisplayInfo.display_w, DisplayInfo.display_h, clear_color,
              window);
  }

  exit_main(window);
}
