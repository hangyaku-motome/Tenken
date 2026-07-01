#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <fstream>
#include <nlohmann/json.hpp>

#include "DataInspectorW.h"
#include "display.h"
#include "FavouriteList.h"
#include "FavouriteW.h"
#include "HexW.h"
#include "HitList.h"
#include "HitsW.h"
#include "LogW.h"
#include "MapPopUp.h"
#include "scan_ops.h"
#include "Scanner.h"
#include "SearchW.h"
#include "TargetPopUp.h"
#include "types.h"
#include "utils.h"

#include "Platform.h"

using nlohmann::json;

void ResolveActions(Scanner& ScannerObj,
                    const std::vector<PendingAction>& Actions,
                    SessionState& State,
                    std::thread& scannerThread,
                    HitList& Hit,
                    FavouriteList& Favourite);

// TODO: Implement or import a file location chooser.

// TODO: The search window is incredibly wonky. Fix it. A variable for it in state? really? also many UI problems.
//
// WARNING: The windows version will definitely fail as of this latest version. I'm gonna have to get around to fixing
// it...At some point. For now (hopefully) that ugly ifndef ifdef piece keeps windows working.

// Hard coded save location for now.
int saveTenken(std::filesystem::path savePath, const std::vector<FavouriteInfoT>& favourites);
int loadTenken(std::filesystem::path savePath, std::vector<FavouriteInfoT>& favourites);

int main() {
  if (Platform::checkPermission() == false) {
    printf("Please give the necessary permissions to run this program. Consult the README for details.\n");
    return 1;
  }

  // Start up Dear ImGui.
  std::filesystem::path ImGuiInitPath = Platform::getImGuiInitPath();
  GLFWwindow* window = initalise_imgui(ImGuiInitPath);
  ImGuiIO& io = ImGui::GetIO();
  ImVec4 clear_color = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);

  // Start up Tenken.
  std::filesystem::path savePath = Platform::getSavePath();

  SessionState State;

  Scanner ScannerObj;

  SearchW SearchWObj;
  HitsW HitWObj;
  FavouriteW FavouriteWObj;
  LogW LogWObj;
  HexW HexWObj(ScannerObj);
  DataInspectorW DataInspectorWObj(ScannerObj);

  TargetPopUp TargetPUpObj;
  MapsPopUp MapPUpObj;

  HitList Hit;
  FavouriteList Favourite;

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

    std::vector<PendingAction> Actions;

    std::string menuBarAction = MainMenuBarCycle(
        TargetPUpObj.clicked_, MapPUpObj.clicked_, LogWObj.enabled_, HexWObj.enabled_, DataInspectorWObj.enabled_);

    if (menuBarAction == "Save") {
      if (!saveTenken(savePath, Favourite.get())) Log::Info("Save successful.");
    }
    if (menuBarAction == "Load") {
      std::vector<FavouriteInfoT> newFavourites;
      if (!loadTenken(savePath, newFavourites))
        Favourite.assignNew(newFavourites);
      else
        Log::Error("Load failed\n");
    }

    // Target popup.
    Actions.push_back(TargetPUpObj.CyclePUp());

    // Region popup.
    if (MapPUpObj.refresh_) MapPUpObj.UpdateRegions(ScannerObj.getMapRegions());
    MapPUpObj.CyclePUp();

    // Hits window.
    if (!State.IsScanning)
      Actions.push_back(HitWObj.CycleW(Hit.getAll(), State));
    else
      Actions.push_back(HitWObj.CycleW({}, State));

    // Search window.
    Actions.push_back(SearchWObj.CycleW(State.TargetInfo, State.SearchWStatus, State.IsUnknownnValueScan));

    // Favourite window.
    Actions.push_back(FavouriteWObj.CycleW(Favourite.get(), State));

    // Log window.
    LogWObj.CycleW();

    // Hex window.
    HexWObj.CycleW();

    // Data Inspector window.
    DataInspectorWObj.CycleW();

    end_frame(static_cast<int32_t>(io.DisplaySize.x), static_cast<int32_t>(io.DisplaySize.y), clear_color, window);

    // resolve actions.
    ResolveActions(ScannerObj, Actions, State, scannerThread, Hit, Favourite);
    Actions.clear();

    // regular refresh fav
    if (State.favRefreshSeconds >= 0.3) {
      auto favouriterefresh = std::chrono::duration_cast<std::chrono::milliseconds>(LoopTime - FavouriteRefreshTime);
      if (favouriterefresh.count() >= static_cast<int64_t>(State.favRefreshSeconds * 1000)) {
        Favourite.rescanAll(ScannerObj, State.TargetInfo.TargetType);
        FavouriteRefreshTime = LoopTime;
      }
    }

    // regular refresh hit
    if (State.hitRefreshSeconds >= 0.3) {
      auto hitrefresh = std::chrono::duration_cast<std::chrono::milliseconds>(LoopTime - HitRefreshTime);
      if (hitrefresh.count() >= static_cast<int64_t>(State.hitRefreshSeconds * 1000)) {
        ScanOp::RunOnScannerThread(scannerThread, State, [&]() {
          ScanOp::rescanAllHits(ScannerObj, Hit, State.ScanProgress, State.TargetInfo.TargetType);
        });
        HitRefreshTime = LoopTime;
      }
    }
  }
  exit_main(window);
  Favourite.endFreezeThread();
  if (scannerThread.joinable()) scannerThread.join();
}

void ResolveActions(Scanner& ScannerObj,
                    const std::vector<PendingAction>& Actions,
                    SessionState& State,
                    std::thread& scannerThread,
                    HitList& Hit,
                    FavouriteList& Favourite) {
  for (auto& Pending : Actions) {
    std::visit(
        overloaded{
            [&](const Action::TargetProcChosen& a) {
              Hit.reset();
              Favourite.reset();
              ScannerObj.init(a.chosenProc.pid);
              State.TargetProcInfo = a.chosenProc;
              State.SearchWStatus = SessionState::SearchWStatusT::FIRST;
              State.TargetChosen = true;
              Favourite.startFreezeThread(ScannerObj);
            },
            [&](const Action::firstScan) {
              ScanOp::RunOnScannerThread(scannerThread, State, [&]() {
                auto hits = ScanOp::startScan(ScannerObj, State);
                Hit.assignNew(std::move(hits));
              });
              State.SearchWStatus = SessionState::SearchWStatusT::SECOND;
            },
            [&](const Action::startUnknownValueScan) {
              State.IsUnknownnValueScan = true;
              ScanOp::RunOnScannerThread(scannerThread, State, [&]() {
                State.Snapshots = ScannerObj.StartUnknownValueScan(State.ScanProgress);
              });
              State.SearchWStatus = SessionState::SearchWStatusT::SECOND;
            },
            [&](const Action::filterByValue& a) {
              ScanOp::RunOnScannerThread(scannerThread, State, [&, value = a.value]() {
                ScanOp::rescanAllHits(ScannerObj, Hit, State.ScanProgress, State.TargetInfo.TargetType);
                Hit.filter(value);
              });
            },
            [&](const Action::filterByStatus& a) {
              if (State.IsUnknownnValueScan) {
                ScanOp::RunOnScannerThread(scannerThread, State, [&, status = a.status]() {
                  Hit.assignNew(ScannerObj.FilterSnapshots(State.Snapshots, status, State.TargetInfo.TargetType));
                  State.IsUnknownnValueScan = false;
                  State.Snapshots = {};
                });
              } else {
                ScanOp::RunOnScannerThread(scannerThread, State, [&, status = a.status]() {
                  ScanOp::rescanAllHits(ScannerObj, Hit, State.ScanProgress, State.TargetInfo.TargetType);
                  Hit.filter(status);
                });
              }
            },
            [&](const Action::writeHit& a) {
              Hit.write(ScannerObj, a.index, a.value);
              Hit.rescan(ScannerObj, a.index, State.TargetInfo.TargetType);
            },
            [&](const Action::rescanHit& a) { Hit.rescan(ScannerObj, a.index, State.TargetInfo.TargetType); },
            [&](const Action::rescanAllHits) {
              ScanOp::RunOnScannerThread(scannerThread, State, [&]() {
                ScanOp::rescanAllHits(ScannerObj, Hit, State.ScanProgress, State.TargetInfo.TargetType);
              });
            },
            [&](const Action::regularRefreshHits& a) { State.hitRefreshSeconds = a.seconds; },

            // start of favourite stuff.
            [&](const Action::addFavourite& a) {
              Favourite.add(Hit.getIndex(a.hitIndex), State.TargetInfo.TargetType);
            },
            [&](const Action::removeFavourite& a) { Favourite.remove(a.index); },
            [&](const Action::writeFavourite& a) {
              Favourite.write(ScannerObj, a.index, a.value);
              Favourite.rescan(ScannerObj, a.index, State.TargetInfo.TargetType);
            },
            [&](const Action::isFreezeFavourite& a) { Favourite.setFreeze(a.index, a.freeze); },
            [&](const Action::freezeValueFavourite& a) { Favourite.setFreezeVal(a.index, a.value); },
            [&](const Action::descFavourite& a) { Favourite.setDesc(a.index, a.value); },
            [&](const Action::rescanFavourite& a) {
              Favourite.rescan(ScannerObj, a.index, State.TargetInfo.TargetType);
            },
            [&](const Action::regularRefreshFavourite& a) { State.favRefreshSeconds = a.seconds; },
            [&](const Action::rescanAllFavourites) { Favourite.rescanAll(ScannerObj, State.TargetInfo.TargetType); },
            // end of favourite stuff.

            [&](const Action::restartScan) {
              Hit.reset();
              State.favRefreshSeconds = -1;
              State.hitRefreshSeconds = -1;
              State.TargetInfo.TargetType = TargetTypeT::Invalid;
              State.TargetInfo.value = {};
              State.SearchWStatus = SessionState::SearchWStatusT::FIRST;
            },
            [&](const Action::setTargetInfo& a) {
              State.TargetInfo.TargetType = a.type;
              State.TargetInfo.value = a.value;
              if (a.mask.has_value()) State.TargetInfo.mask = a.mask;
            },
            [&](const Action::undoScan) { Hit.RestoreOldHits(); },
            [&](const std::monostate&) {}},
        Pending);
  }
}

int saveTenken(std::filesystem::path savePath, const std::vector<FavouriteInfoT>& favourites) {
  try {
    json savedState;
    savedState["favourites"] = json::array();
    for (const auto& favourite : favourites) {
      json item;

      std::stringstream locationStream;

      locationStream << std::hex << std::showbase << favourite.location;
      item["location"] = locationStream.str();
      item["value"] = favourite.value;  // raw bytes for now. should be fine.
      item["desc"] = favourite.desc;
      item["type"] = targetTypeToStr(favourite.type);

      savedState["favourites"].push_back(item);
    }
    savedState["version"] = 1;

    std::filesystem::create_directories(savePath.parent_path());
    std::ofstream saveFile(savePath);
    saveFile << savedState.dump(2);

  } catch (...) {
    printf("failed to save state.\n");
    return 1;
  }
  return 0;
}

int loadTenken(std::filesystem::path savePath, std::vector<FavouriteInfoT>& favourites) {
  try {
    json loadedState;

    std::ifstream loadFile(savePath);

    if (!loadFile) {
      Log::Error("Failed to open save from path. Are you sure it exists?");
      return 1;
    }

    loadedState = json::parse(loadFile);

    if (loadedState.value("version", 0) != 1) {
      Log::Error(
          "Expected version and current version do not match for save file. Are you on a newer/older version than "
          "when "
          "you saved? If not...Yeah IDK what happened something is wrong clearly. Here is the supposed version" +
          std::to_string(loadedState.value("version", 0)) +
          " . And the only reason this log file is so unnecessarily long is because I felt like it. Anyways good "
          "luck "
          "with this problem someone who is probably me.");
      return 1;
    }

    std::vector<FavouriteInfoT> newFavourites;
    for (const auto& item : loadedState.at("favourites")) {
      FavouriteInfoT favourite;

      favourite.desc = item.at("desc").get<std::string>();
      favourite.location = std::stoull(item.at("location").get<std::string>(), nullptr, 16);

      std::vector<uint8_t> value;
      for (const auto& byte : item.at("value")) {
        value.push_back(byte);
      }
      favourite.value = value;
      favourite.type = strToTargetType(item["type"].get<std::string>());

      newFavourites.push_back(favourite);
    }
    favourites = std::move(newFavourites);
  } catch (...) {
    Log::Error("Failed to load. idk why.");
    return 1;
  }
  return 0;
}
