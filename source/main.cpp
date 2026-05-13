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
#include <string>

OpType ResolveActions(PendingAction Actions, Scanner &ScannerObj,
                      TargetInfoT &TargetInfo);

int main() {

  // Start up Dear ImGui.
  GLFWwindow *window = initalise_main();
  ImVec4 clear_color = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  TargetInfoT TargetInfo;
  ProcessInfoT TargetProc;

  DisplayInfoT DisplayInfo{};

  SearchW SearchObj;
  HitsW HitObj;
  TargetPopUp TargetPUpObj;
  FavouriteW FavouriteObj;

  Scanner ScannerObj;

  PendingAction Actions{};
  bool TargetProcChosen = false;

  // Main loop.
  while (glfwWindowShouldClose(window) == 0) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }
    const auto LoopTime = std::chrono::steady_clock::now();

    start_frame();
    SetDefaultDisplay();

    MainMenuBarCycle(TargetPUpObj);

    // Target popup.
    Actions.OverrideType = TargetPUpObj.CyclePUp(TargetProc);
    if (Actions.OverrideType == OpType::INIT_SCANNER) {
      SearchObj = {};
      HitObj = {};
      TargetInfo = {};
      ScannerObj.FullClear();
      TargetProcChosen = true;
      ScannerObj.StartFreezeThread();
    }

    // Hits window.
    float ProgressBar = -1;
    if (ScannerObj.IsScanning) {
      ProgressBar = static_cast<float>(ScannerObj.CurrentProgress) /
                    static_cast<float>(ScannerObj.TotalLoad);
      Actions.HitW = HitObj.CycleW({}, TargetInfo, ProgressBar);
    } else
      Actions.HitW = HitObj.CycleW(ScannerObj.Hits, TargetInfo, ProgressBar);

    // Search window.
    Actions.SearchW = (SearchObj.CycleW(TargetInfo, TargetProcChosen));

    // Favourite window.
    Actions.FavouriteW = (FavouriteObj.CycleW(ScannerObj.GetFavourites()));

    // Log window.
    LogW::CycleW();

    end_frame(DisplayInfo.display_w, DisplayInfo.display_h, clear_color,
              window);

    // resolve actions.
    switch (ResolveActions(Actions, ScannerObj, TargetInfo)) {
    case OpType::INIT_SCANNER:
      ScannerObj.Init(TargetProc.pid);
      break;
    case OpType::RESTART_STATE:
      HitObj = {};
      ScannerObj.RestartClear();
      TargetProcChosen = true;
      ScannerObj.StartFreezeThread();
      TargetInfo = {};
      SearchObj = {};
      break;
    default:
      break;
    }

    Actions = {};

    ScannerObj.AutoRefreshHits(LoopTime, TargetInfo.TargetType);
    ScannerObj.AutoRefreshFavourites(LoopTime);
  }
  exit_main(window);
}

OpType ResolveActions(PendingAction Actions, Scanner &ScannerObj,
                      TargetInfoT &TargetInfo) {

  switch (Actions.OverrideType) {
  case OpType::INIT_SCANNER:
    return Actions.OverrideType;
  default:
    break;
  }

  switch (Actions.HitW.Type) {
  case OpType::EDIT:
    if (TargetInfo.value.size() < Actions.HitW.buf->size())
      Actions.HitW.buf->resize(TargetInfo.value.size());
    ScannerObj.WriteHit(Actions.HitW.index.value(), Actions.HitW.buf.value());
    ScannerObj.RescanHit(Actions.HitW.index.value(), TargetInfo.TargetType);

    break;
  case OpType::REFRESH:
    if (Actions.HitW.index.has_value())
      ScannerObj.RescanHit(Actions.HitW.index.value(), TargetInfo.TargetType);
    else
      ScannerObj.RunOnScannerThread([&TargetInfo](Scanner &s) {
        s.RescanAllHits(TargetInfo.TargetType);
      });
    break;
  case OpType::REGULAR_REFRESH:
    if (Actions.HitW.seconds.value() == -1 || Actions.HitW.seconds.value() == 0)
      ScannerObj.HitRefreshInterval =
          static_cast<int64_t>(Actions.HitW.seconds.value());
    else
      ScannerObj.HitRefreshInterval =
          static_cast<int64_t>(Actions.HitW.seconds.value() * 1000) >= 300
              ? static_cast<int64_t>(Actions.HitW.seconds.value() * 1000)
              : 300;
    break;
  case OpType::ADD_TO_FAVOURITES:
    ScannerObj.AddToFavourite(Actions.HitW.index.value(),
                              TargetInfo.TargetType);

  default:
    break;
  }

  switch (Actions.FavouriteW.Type) {
  case OpType::EDIT:
    if (Actions.FavouriteW.newname.has_value()) {
      ScannerObj.SetDescriptionFavourite(Actions.FavouriteW.index.value(),
                                         Actions.FavouriteW.newname.value());
      break;
    }
    ScannerObj.WriteFavourite(Actions.FavouriteW.index.value(),
                              Actions.FavouriteW.buf.value());
    ScannerObj.RescanFavourite(Actions.FavouriteW.index.value());
    break;
  case OpType::FREEZE:
    ScannerObj.SetFreezeFavourite(Actions.FavouriteW.index.value(), true);
    break;
  case OpType::UNFREEZE:
    ScannerObj.SetFreezeFavourite(Actions.FavouriteW.index.value(), false);
    break;
  case OpType::REFRESH:
    if (Actions.FavouriteW.index.has_value())
      ScannerObj.RescanFavourite(Actions.FavouriteW.index.value());
    else
      ScannerObj.RescanAllFavourites();
    break;
  case OpType::REMOVE_FROM_FAVOURITES:
    ScannerObj.RemoveFromFavourite(Actions.FavouriteW.index.value());
    break;
  case OpType::REGULAR_REFRESH:
    ScannerObj.SetRefreshDurationFavourite(Actions.FavouriteW.index.value(),
                                           Actions.FavouriteW.seconds.value());
  default:
    break;
  }

  switch (Actions.SearchW.Type) {
  case OpType::FIRST_SCAN:
    Log::Info("Starting initial scan...");
    ScannerObj.RunOnScannerThread(
        [&TargetInfo](Scanner &s) { s.StartScan(TargetInfo); });
    break;
  case OpType::FILTER:
    if (Actions.SearchW.KeepType.has_value())
      ScannerObj.RunOnScannerThread([&TargetInfo, &Actions](Scanner &s) {
        s.RescanAllHits(TargetInfo.TargetType);
        s.FilterHit(Actions.SearchW.KeepType.value());
      });
    else
      ScannerObj.RunOnScannerThread([&TargetInfo](Scanner &s) {
        s.RescanAllHits(TargetInfo.TargetType);
        s.FilterHit(TargetInfo.value);
      });
    break;
  case OpType::RESTART_STATE:
    return OpType::RESTART_STATE;
  default:
    break;
  }
  return {};
}