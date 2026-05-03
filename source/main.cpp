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

// Put window dimensions in objects. And have a single function called at the
// start that recalculates it all *if* dimensions change. Also we should NOT try
// to put the calculation inside the objects. Sounds dumb. Pass value only.

// A function that refreshes Hit values and bytes regularly in sccanner?
int main() {

  // Start up Dear ImGui.
  GLFWwindow *window = initalise_main();
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // this might as well be a global constexpr, no?
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

    // yeah seriously we might want to move display information inside windows
    // themselves AND flags instead of giving them as a flag every time. It is a
    // required pass that is only relevant to them.
    SetDisplayInfo(window, DisplayInfo);

    MainMenuBarCycle(TargetPUp);

    // target popup.
    if (TargetPUp.CyclePUp(ActiveInfo) ==
        "new target") { // {} should be correct usage here, I'd hope. I'm
                        // assuming it completely resets state and data. // Now
                        // that I think about it if we do this after setting
                        // display info, AND we add the screen sizes into each
                        // objects then, wouldn't all windows disappear for a
                        // single frame?
      SearchObj = {};
      HitObj = {};
      ActiveInfo.TargetValInfo = {};
      ScannerObj = {};
    }

    // Hits.
    HitObj.CycleW(DisplayInfo.Hit, flagsWindowDefault, ScannerObj.Hits,
                  ActiveInfo.TargetValInfo);

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
