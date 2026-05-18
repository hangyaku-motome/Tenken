#include "FavouriteList.h"
#include "FavouriteW.h"
#include "HitList.h"
#include "HitsW.h"
#include "LogW.h"
#include "MapPopUp.h"
#include "Scanner.h"
#include "SearchW.h"
#include "display.h"
#include "scan_ops.h"
#include "types.h"
#include <cstdint>
#include <imgui_impl_glfw.h>
#include <thread>
#include <vector>

void ResolveActions(Scanner &ScannerObj,
                    const std::vector<PendingAction> &Actions,
                    SessionState &State, std::thread &scannerThread,
                    HitList &Hit, FavouriteList &Favourite);

int main() {

  // Start up Dear ImGui.
  GLFWwindow *window = initalise_main();
  ImVec4 clear_color = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  SessionState State;

  SearchW SearchWObj;
  HitsW HitWObj;
  FavouriteW FavouriteWObj;

  TargetPopUp TargetWObj;
  MapsPopUp MapWObj;

  Scanner ScannerObj;

  HitList Hit;
  FavouriteList Favourite;

  std::vector<PendingAction> Actions;

  std::thread scannerThread;

  auto HitRefreshTime = std::chrono::steady_clock::now();
  auto FavouriteRefreshTime = std::chrono::steady_clock::now();

  // Main loop.
  while (glfwWindowShouldClose(window) == 0) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }
    const auto LoopTime = std::chrono::steady_clock::now();

    start_frame();
    SetDefaultDisplay();

    MainMenuBarCycle(TargetWObj, MapWObj);

    // Target popup.
    Actions.push_back(TargetWObj.CyclePUp());

    // Region popup.
    if (MapWObj.refresh_)
      MapWObj.UpdateRegions(ScannerObj.getMapRegions());
    MapWObj.CyclePUp();

    // Hits window.
    if (!State.IsScanning)
      Actions.push_back(HitWObj.CycleW(Hit.getAll(), State));
    else
      Actions.push_back(HitWObj.CycleW({}, State));

    // Search window.
    Actions.push_back(SearchWObj.CycleW(State.TargetInfo, State.searchW,
                                        State.IsUnknownnValueScan));

    // Favourite window.
    Actions.push_back(FavouriteWObj.CycleW(Favourite.get(), State));

    // Log window.
    LogW::CycleW();

    end_frame(static_cast<int32_t>(io.DisplaySize.x),
              static_cast<int32_t>(io.DisplaySize.y), clear_color, window);

    // resolve actions.
    ResolveActions(ScannerObj, Actions, State, scannerThread, Hit, Favourite);
    Actions.clear();

    // regular refresh fav
    if (State.favRefreshSeconds >= 0.3) {
      auto favouriterefresh =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              LoopTime - FavouriteRefreshTime);
      if (favouriterefresh.count() >=
          static_cast<int64_t>(State.favRefreshSeconds * 1000)) {
        Favourite.rescanAll(ScannerObj, State.TargetInfo.TargetType);
        FavouriteRefreshTime = LoopTime;
      }
    }

    // regular refresh hit
    if (State.hitRefreshSeconds >= 0.3) {
      auto hitrefresh = std::chrono::duration_cast<std::chrono::milliseconds>(
          LoopTime - HitRefreshTime);
      if (hitrefresh.count() >=
          static_cast<int64_t>(State.hitRefreshSeconds * 1000)) {
        ScanOp::RunOnScannerThread(scannerThread, State, [&]() {
          ScanOp::rescanAllHits(ScannerObj, Hit, State.ScanProgress,
                                State.TargetInfo.TargetType);
        });
        HitRefreshTime = LoopTime;
      }
    }
  }
  exit_main(window);
  Favourite.endFreezeThread();
  if (scannerThread.joinable())
    scannerThread.join();
}

void ResolveActions(Scanner &ScannerObj,
                    const std::vector<PendingAction> &Actions,
                    SessionState &State, std::thread &scannerThread,
                    HitList &Hit, FavouriteList &Favourite) {

  for (auto &Pending : Actions) {
    std::visit(
        overloaded{
            [&](const Action::TargetProcChosen &a) {
              Hit.reset();
              Favourite.reset();
              ScannerObj.init(a.chosenProc.pid);
              State.TargetProcInfo = a.chosenProc;
              State.searchW = SessionState::SearchWStatus::FIRST;
              State.TargetChosen = true;
              Favourite.startFreezeThread(ScannerObj);
            },
            [&](const Action::firstScan) {
              ScanOp::RunOnScannerThread(scannerThread, State, [&]() {
                auto hits = ScanOp::startScan(ScannerObj, State);
                Hit.assignNew(std::move(hits));
              });
              State.searchW = SessionState::SearchWStatus::SECOND;
            },
            [&](const Action::startUnknownValueScan) {
              State.IsUnknownnValueScan = true;
              ScanOp::RunOnScannerThread(scannerThread, State, [&]() {
                State.Snapshots =
                    ScannerObj.StartUnknownValueScan(State.ScanProgress);
              });
              State.searchW = SessionState::SearchWStatus::SECOND;
            },
            [&](const Action::filterByValue &a) {
              ScanOp::RunOnScannerThread(
                  scannerThread, State, [&, value = a.value]() {
                    ScanOp::rescanAllHits(ScannerObj, Hit, State.ScanProgress,
                                          State.TargetInfo.TargetType);
                    Hit.filter(value);
                  });
            },
            [&](const Action::filterByStatus &a) {
              if (State.IsUnknownnValueScan) {
                ScanOp::RunOnScannerThread(
                    scannerThread, State, [&, status = a.status]() {
                      Hit.assignNew(ScannerObj.FilterSnapshots(
                          State.Snapshots, status,
                          State.TargetInfo.TargetType));
                      State.IsUnknownnValueScan = false;
                      State.Snapshots = {};
                    });
              } else {
                ScanOp::RunOnScannerThread(
                    scannerThread, State, [&, status = a.status]() {
                      ScanOp::rescanAllHits(ScannerObj, Hit, State.ScanProgress,
                                            State.TargetInfo.TargetType);
                      Hit.filter(status);
                    });
              }
            },
            [&](const Action::writeHit &a) {
              Hit.write(ScannerObj, a.index, a.value);
              Hit.rescan(ScannerObj, a.index, State.TargetInfo.TargetType);
            },
            [&](const Action::rescanHit &a) {
              Hit.rescan(ScannerObj, a.index, State.TargetInfo.TargetType);
            },
            [&](const Action::rescanAllHits) {
              ScanOp::RunOnScannerThread(scannerThread, State, [&]() {
                ScanOp::rescanAllHits(ScannerObj, Hit, State.ScanProgress,
                                      State.TargetInfo.TargetType);
              });
            },
            [&](const Action::regularRefreshHits &a) {
              State.hitRefreshSeconds = a.seconds;
            },
            // start of favourite stuff.
            [&](const Action::addFavourite &a) {
              Favourite.add(Hit.getIndex(a.hitIndex),
                            State.TargetInfo.TargetType);
            },
            [&](const Action::removeFavourite &a) {
              Favourite.remove(a.index);
            },
            [&](const Action::writeFavourite &a) {
              Favourite.write(ScannerObj, a.index, a.value);
              Favourite.rescan(ScannerObj, a.index,
                               State.TargetInfo.TargetType);
            },
            [&](const Action::isFreezeFavourite &a) {
              Favourite.setFreeze(a.index, a.freeze);
            },
            [&](const Action::freezeValueFavourite &a) {
              Favourite.setFreezeVal(a.index, a.value);
            },
            [&](const Action::descFavourite &a) {
              Favourite.setDesc(a.index, a.value);
            },
            [&](const Action::rescanFavourite &a) {
              Favourite.rescan(ScannerObj, a.index,
                               State.TargetInfo.TargetType);
            },
            [&](const Action::regularRefreshFavourite &a) {
              State.favRefreshSeconds = a.seconds;
            },
            [&](const Action::rescanAllFavourites) {
              Favourite.rescanAll(ScannerObj, State.TargetInfo.TargetType);
            },
            // end of favourite stuff.
            [&](const Action::restartScan) {
              Hit.reset();
              State.favRefreshSeconds = -1;
              State.hitRefreshSeconds = -1;
              State.TargetInfo.TargetType = TargetTypeT::Invalid;
              State.TargetInfo.value = {};
              State.searchW = SessionState::SearchWStatus::FIRST;
            },
            [&](const Action::setTargetInfo &a) {
              State.TargetInfo.TargetType = a.type;
              State.TargetInfo.value = a.value;
            },
            [&](const std::monostate &) {}},
        Pending);
  }
}