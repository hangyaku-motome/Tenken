#include "Favourite.h"
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

int main() {

  // Start up Dear ImGui.
  GLFWwindow *window = initalise_main();
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  TargetInfoT TargetInfo;
  ProcessInfoT TargetProc;

  DisplayInfoT DisplayInfo;

  LogW LogObj;
  SearchW SearchObj;
  HitsW HitObj;
  TargetPopUp TargetPUpObj;
  FavouriteW FavouriteObj;

  Scanner ScannerObj;

  std::vector<Action> PendingActions;
  bool TargetProcChosen = false;

  // Main loop.
  while (!glfwWindowShouldClose(window)) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    start_frame();

    SetDefaultDisplay();

    MainMenuBarCycle(TargetPUpObj);

    // Target popup.
    PendingActions.push_back(Action{TargetPUpObj.CyclePUp(TargetProc)});
    if (PendingActions.back().Type == OpType::INIT_SCANNER) {
      SearchObj = {};
      HitObj = {};
      TargetInfo = {};
      ScannerObj = {};
      TargetProcChosen = true;
    }

    // Hits window.
    PendingActions.push_back(HitObj.CycleW(ScannerObj.Hits, TargetInfo));

    // Search window.
    PendingActions.push_back(SearchObj.CycleW(TargetInfo, TargetProcChosen));

    // Favourite window.
    PendingActions.push_back(
        FavouriteObj.CycleW(ScannerObj.Favourites, TargetInfo.TargetType));

    // Log window.
    LogObj.CycleW();

    end_frame(DisplayInfo.display_w, DisplayInfo.display_h, clear_color,
              window);

    // resolve actions.
    // oh...we can have resolve actions be on a thread. Later, though.
    // also this doesn't exactly work. we should prolly pop and work on it,
    // instead of clearing at the end. IDK we'll see.

    for (auto const &Action : PendingActions) {
      switch (Action.Type) {
      case OpType::NONE:
        continue;
      case OpType::INIT_SCANNER:
        ScannerObj.Init(TargetProc.pid);
        continue;
      case OpType::ADD_TO_FAVOURITES:
        if (!Action.index.has_value()) {
          Log::Error("In PendingActions, why does add to favourites exist but "
                     "no index?");
          continue;
        }
        printf("okay I try to add to favourites.\n");
        ScannerObj.AddToFavourite(Action.index.value());
        continue;
      case OpType::FIRST_SCAN:
        Log::Info("Starting initial scan...");
        ScannerObj.StartScan(TargetInfo);
        continue;
      case OpType::REMOVE_FROM_FAVOURITES:
        continue;
      case OpType::REFRESH_ALL_HITS:
        for (uint64_t i = 0; i < ScannerObj.Hits.size(); ++i)
          ScannerObj.RescanHit(i, TargetInfo);
        continue;
      case OpType::EDIT_HIT:
        if (!Action.newval.has_value()) {
          Log::Error(
              "In PendingActions, why does edit hit exist but no new value?");
          continue;
        }
        ScannerObj.WriteAdr(Action.index.value(), Action.newval.value());
        ScannerObj.RescanHit(Action.index.value(), TargetInfo);
        // we might also wanna refresh bytes of the other hits. not their
        // values! just their bytes.
        // that reminds me, we should tag changed bytes. so...we'd need either
        // another vector of bytes or a vector of bools changed/unchaged for
        // each byte.
        continue;
      case OpType::RESCAN:
        printf("hi you tried to rescan\n");
        if (!Action.BasedOnCurrentValues.value())
          for (uint64_t i = 0; i < ScannerObj.Hits.size(); ++i)
            ScannerObj.RescanHit(i, TargetInfo);
        if (Action.KeepType.has_value())
          ScannerObj.FilterHit(Action.KeepType.value());
        else
          ScannerObj.FilterHit(TargetInfo.value);
        continue;
      case OpType::REFRESH_HIT:
        if (!Action.index.has_value()) {
          Log::Error(
              "In PendingActions, why does refresh hit exist but no index?");
          continue;
        }
        ScannerObj.RescanHit(Action.index.value(), TargetInfo);
        continue;
      case OpType::REFRESH_FAVOURITES:
      case OpType::REFRESH_ALL_FAVOURITES:
        break;
      }
    }
    PendingActions.clear();
  }
  exit_main(window);
}
