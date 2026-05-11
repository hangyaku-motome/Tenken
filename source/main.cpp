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

  std::vector<Action> PendingActions{};
  bool TargetProcChosen = false;

  // Main loop.
  while (!glfwWindowShouldClose(window)) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }
    const auto LoopTime = std::chrono::steady_clock::now();

    start_frame();
    SetDefaultDisplay();

    MainMenuBarCycle(TargetPUpObj);

    // Target popup.
    PendingActions.push_back(Action{TargetPUpObj.CyclePUp(TargetProc)});
    if (PendingActions.back().Type == OpType::INIT_SCANNER) {
      SearchObj = {};
      HitObj = {};
      TargetInfo = {};
      ScannerObj.FullClear();
      TargetProcChosen = true;
      ScannerObj.StartFreezeThread();
    }

    float ProgressBar = -1;
    if (ScannerObj.IsScanning) {
      if (ScannerObj.TotalLoad >= 0 && ScannerObj.CurrentProgress >= 0)
        ProgressBar =
            (float)ScannerObj.CurrentProgress / (float)ScannerObj.TotalLoad;
    }

    // Hits window.
    PendingActions.push_back(
        HitObj.CycleW(ScannerObj.Hits, TargetInfo, ProgressBar));
    // Search window.
    PendingActions.push_back(SearchObj.CycleW(TargetInfo, TargetProcChosen));

    // Favourite window.
    PendingActions.push_back(FavouriteObj.CycleW(ScannerObj.GetFavourites()));
    // Log window.
    // should also tell found hit count on rescans and how many were filtered.
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
        ScannerObj.AddToFavourite(Action.index.value(), TargetInfo.TargetType);
        continue;
      case OpType::FIRST_SCAN: // super heavy
        Log::Info("Starting initial scan...");
        ScannerObj.RunOnScannerThread(
            [&TargetInfo](Scanner &s) { s.StartScan(TargetInfo); });
        continue;
      case OpType::REMOVE_FROM_FAVOURITES:
        continue;
      case OpType::REFRESH_ALL: // kinda heavy.
        if (Action.WorkOn == DataType::HIT)
          ScannerObj.RunOnScannerThread([&TargetInfo](Scanner &s) {
            s.RescanAllHits(TargetInfo.TargetType);
          });
        else if (Action.WorkOn == DataType::FAVOURITE)
          for (uint64_t i = 0; i < ScannerObj.GetFavourites().size(); ++i)
            ScannerObj.RescanFavourite(i);
        continue;
      case OpType::EDIT:
        if (!Action.newval.has_value()) {
          Log::Error(
              "In PendingActions, why does edit hit exist but no new value?");
          continue;
        }
        if (Action.WorkOn == DataType::HIT) {
          ScannerObj.WriteHit(Action.index.value(), Action.newval.value());
          ScannerObj.RescanHit(Action.index.value(), TargetInfo.TargetType);
        } else if (Action.WorkOn == DataType::FAVOURITE) {
          ScannerObj.WriteFavourite(Action.index.value(),
                                    Action.newval.value());
          ScannerObj.RescanFavourite(Action.index.value());
        }
        continue;
      case OpType::FILTER: // heavy.
        if (Action.WorkOn != DataType::HIT)
          continue;
        ScannerObj.RunOnScannerThread([&TargetInfo](Scanner &s) {
          s.RescanAllHits(TargetInfo.TargetType);
        });
        if (Action.KeepType.has_value())
          ScannerObj.FilterHit(Action.KeepType.value());
        else
          ScannerObj.FilterHit(TargetInfo.value);
        continue;
      case OpType::REFRESH:
        if (!Action.index.has_value()) {
          // hit this.
          Log::Error("In PendingActions, why does refresh exist but no index?");
          continue;
        }
        if (Action.WorkOn == DataType::HIT)
          ScannerObj.RescanHit(Action.index.value(), TargetInfo.TargetType);
        else if (Action.WorkOn == DataType::FAVOURITE)
          ScannerObj.RescanFavourite(Action.index.value());
        continue;
      case OpType::FREEZE:
        if (!Action.index.has_value()) {
          Log::Error("In action freeze, no index value");
          continue;
        }
        ScannerObj.SetFreezeFavourite(Action.index.value(), true);

        continue;
      case OpType::UNFREEZE:
        ScannerObj.SetFreezeFavourite(Action.index.value(), false);
        continue;
      case OpType::CHANGE_NAME:
        if (!Action.index.has_value()) {
          printf("sdfasfsad\n");
          continue;
        }
        if (!Action.newname.has_value()) {
          printf("stinkly\n");
        }
        ScannerObj.SetDescriptionFavourite(Action.index.value(),
                                           Action.newname.value());
        continue;
      case OpType::REGULAR_REFRESH:
        if (Action.WorkOn == DataType::HIT) {
          if (Action.seconds.value() == -1) {
            ScannerObj.HitRefreshInterval = -1;
            continue;
          }
          if (Action.seconds.value() == 0) {
            ScannerObj.HitRefreshInterval = 0;
            continue;
          }
          printf("I came here too!\n");
          ScannerObj.HitRefreshInterval =
              static_cast<uint32_t>(Action.seconds.value() * 1000) >= 300
                  ? static_cast<uint32_t>(Action.seconds.value() * 1000)
                  : 300;
          continue;
        }
        if (Action.WorkOn == DataType::FAVOURITE) {
          printf("okay you want me to regularly refresh favourite %lu. with "
                 "duration %f\n",
                 Action.index.value(), Action.seconds.value());
          ScannerObj.SetRefreshDurationFavourite(Action.index.value(),
                                                 Action.seconds.value());
        }
        continue;
      case OpType::RESTART_STATE:
        HitObj = {};
        ScannerObj.RestartClear();
        TargetProcChosen = true;
        ScannerObj.StartFreezeThread();
        TargetInfo = {};
        SearchObj = {};
        continue;
      }
    }
    PendingActions.clear();

    ScannerObj.AutoRefreshHits(LoopTime, TargetInfo.TargetType);
    ScannerObj.AutoRefreshFavourites(LoopTime);
  }
  exit_main(window);
}