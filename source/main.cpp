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
#include "HitW.h"
#include "LogW.h"
#include "MapPopUp.h"
#include "Platform.h"
#include "scan_ops.h"
#include "Scanner.h"
#include "SearchW.h"
#include "TargetPopUp.h"
#include "types.h"
#include "utils.h"

using nlohmann::json;

void resolveActions(Scanner& scanner_obj,
                    const std::vector<PendingAction>& actions,
                    SessionState& state,
                    std::thread& scanner_thread,
                    HitList& hit,
                    FavouriteList& favourite);

// TODO: Implement or import a file location chooser.

// TODO: My naming sense and rules changed a lot as I have been writing this, so the names are inconsistent. Need to
// address that.

// PascalCase for all struct, enum, and class names.
// camelCase for all function names and iterator names.
// snake_case for all local and struct variables, function variables, and constexpr.
// snake_case_ with trailing underscore for all object variables.
// For now this is the standard I'll start following. hopefully I got most if not all wrong usages here and there.

// Hard coded save location for now. (share/state and share/local or equavilent in windows for save file and imgui.ini)
bool saveTenken(std::filesystem::path save_path, const std::vector<FavouriteInfoT>& favourites);
bool loadTenken(std::filesystem::path save_path, std::vector<FavouriteInfoT>& favourites);

int main() {
  if (Platform::checkPermission() == false) {
    printf("Please give the necessary permissions to run this program. Consult the README for details.\n");
    return 1;
  }

  // Pointer scanning!

  // Start up Dear ImGui.
  std::filesystem::path imgui_init_path = Platform::getImGuiInitPath();
  std::string imgui_init_path_str = imgui_init_path.string();
  GLFWwindow* window = initalise_imgui(imgui_init_path_str);
  ImGuiIO& io = ImGui::GetIO();
  ImVec4 clear_color = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);

  // Start up Tenken.
  std::filesystem::path savePath = Platform::getSavePath();

  SessionState state;

  Scanner scanner;

  SearchW search_w;
  HitW hit_w;
  FavouriteW favourite_w;
  LogW logW;
  HexW hexW(scanner);
  DataInspectorW data_inspector_w(scanner);

  TargetPopUp target_popup;
  MapsPopUp map_popup(scanner);

  HitList hit;
  FavouriteList favourite;

  std::thread scanner_thread;

  auto hit_refresh_time = std::chrono::steady_clock::now();
  auto favourite_refresh_time = std::chrono::steady_clock::now();

  // Main loop.
  while (glfwWindowShouldClose(window) == 0) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }
    const auto loop_time = std::chrono::steady_clock::now();

    start_frame();
    SetDefaultDisplay();

    std::vector<PendingAction> actions;

    std::string menu_bar_action = MainMenuBarCycle(
        target_popup.clicked_, map_popup.clicked_, logW.enabled_, hexW.enabled_, data_inspector_w.enabled_);

    if (menu_bar_action == "Save") {
      if (saveTenken(savePath, favourite.get()) == true)
        Log::info("Save successful.");
      else
        Log::error("Save failed.");
    }
    if (menu_bar_action == "Load") {
      std::vector<FavouriteInfoT> newFavourites;
      if (loadTenken(savePath, newFavourites) == true)
        favourite.assignNew(newFavourites);
      else
        Log::error("Load failed\n");
    }

    // Target popup.
    actions.push_back(target_popup.cyclePopUp());

    // Region popup.
    map_popup.cyclePopUp(state.active_regions);

    // Hits window.
    if (!state.is_scanning)
      actions.push_back(hit_w.cycleW(hit.getAll(), state));
    else
      actions.push_back(hit_w.cycleW({}, state));

    // Search window.
    actions.push_back(search_w.cycleW(state.target_info, state.target_proc_info.pid));

    // Favourite window.
    actions.push_back(favourite_w.cycleW(favourite.get(), state));

    // Log window.
    logW.cycleW();

    // Hex window.
    hexW.cycleW();

    // Data Inspector window.
    data_inspector_w.cycleW();

    end_frame(static_cast<int32_t>(io.DisplaySize.x), static_cast<int32_t>(io.DisplaySize.y), clear_color, window);

    // resolve actions.
    resolveActions(scanner, actions, state, scanner_thread, hit, favourite);
    actions.clear();

    // regular refresh fav
    if (state.fav_refresh_seconds >= 0.3) {
      auto favouriterefresh = std::chrono::duration_cast<std::chrono::milliseconds>(loop_time - favourite_refresh_time);
      if (favouriterefresh.count() >= static_cast<int64_t>(state.fav_refresh_seconds * 1000)) {
        favourite.rescanAll(scanner, state.target_info.target_type);
        favourite_refresh_time = loop_time;
      }
    }

    // regular refresh hit
    if (state.hit_refresh_seconds >= 0.3) {
      auto hitrefresh = std::chrono::duration_cast<std::chrono::milliseconds>(loop_time - hit_refresh_time);
      if (hitrefresh.count() >= static_cast<int64_t>(state.hit_refresh_seconds * 1000)) {
        ScanOp::runOnScannerThread(scanner_thread, state, [&]() {
          ScanOp::rescanAllHits(scanner, hit, state.scan_progress, state.target_info.target_type);
        });
        hit_refresh_time = loop_time;
      }
    }
  }
  exit_imgui(window);
  favourite.endFreezeThread();
  if (scanner_thread.joinable()) scanner_thread.join();
}

void resolveActions(Scanner& scanner_obj,
                    const std::vector<PendingAction>& actions,
                    SessionState& state,
                    std::thread& scanner_thread,
                    HitList& hit,
                    FavouriteList& favourite) {
  for (auto& Pending : actions) {
    std::visit(
        overloaded{
            [&](const Action::TargetProcChosen& a) {
              hit.reset();
              favourite.reset();
              scanner_obj.init(a.chosen_proc.pid);
              state.target_proc_info = a.chosen_proc;
              favourite.startFreezeThread(scanner_obj);
            },
            [&](const Action::FirstScan) {
              ScanOp::runOnScannerThread(scanner_thread, state, [&]() {
                auto hits =
                    ScanOp::startScan(scanner_obj, state.target_info, state.scan_progress, state.active_regions);
                hit.assignNew(std::move(hits));
              });
            },
            [&](const Action::StartUnknownValueScan) {
              state.is_unknown_value_scan =
                  true;  // a way to know if we are doing it actively. it won't need Snapshot after scan 1.
              ScanOp::runOnScannerThread(scanner_thread, state, [&]() {
                state.snapshots = scanner_obj.getSnapshot(state.active_regions, state.scan_progress);
              });
            },
            [&](const Action::FilterByValue& a) {
              ScanOp::runOnScannerThread(scanner_thread, state, [&, value = a.value]() {
                ScanOp::rescanAllHits(scanner_obj, hit, state.scan_progress, state.target_info.target_type);
                hit.filter(value);
              });
            },
            [&](const Action::filterByStatus& a) { if (state.is_unknown_value_scan) {
                ScanOp::runOnScannerThread(scanner_thread, state, [&, status = a.status]() {
                  hit.assignNew(scanner_obj.filterSnapshot(state.snapshots, status, state.target_info.target_type));
                  state.is_unknown_value_scan = false;
                  state.snapshots = {};
                });
              } else {
                ScanOp::runOnScannerThread(scanner_thread, state, [&, status = a.status]() {
                  ScanOp::rescanAllHits(scanner_obj, hit, state.scan_progress, state.target_info.target_type);
                  hit.filter(status);
                });
              }
            },
            [&](const Action::WriteHit& a) {
              hit.write(scanner_obj, a.index, a.value);
              hit.rescan(scanner_obj, a.index, state.target_info.target_type);
            },
            [&](const Action::RescanHit& a) { hit.rescan(scanner_obj, a.index, state.target_info.target_type); },
            [&](const Action::RescanAllHits) {
              ScanOp::runOnScannerThread(scanner_thread, state, [&]() {
                ScanOp::rescanAllHits(scanner_obj, hit, state.scan_progress, state.target_info.target_type);
              });
            },
            [&](const Action::RegularRefreshHits& a) { state.hit_refresh_seconds = a.seconds; },

            // start of favourite stuff.
            [&](const Action::AddFavourite& a) {
              favourite.add(hit.getIndex(a.hitIndex), state.target_info.target_type);
            },
            [&](const Action::RemoveFavourite& a) { favourite.remove(a.index); },
            [&](const Action::WriteFavourite& a) {
              favourite.write(scanner_obj, a.index, a.value);
              favourite.rescan(scanner_obj, a.index, state.target_info.target_type);
            },
            [&](const Action::IsFreezeFavourite& a) { favourite.setFreeze(a.index, a.freeze); },
            [&](const Action::FreezeValueFavourite& a) { favourite.setFreezeVal(a.index, a.value); },
            [&](const Action::DescFavourite& a) { favourite.setDesc(a.index, a.value); },
            [&](const Action::RescanFavourite& a) {
              favourite.rescan(scanner_obj, a.index, state.target_info.target_type);
            },
            [&](const Action::RegularRefreshFavourite& a) { state.fav_refresh_seconds = a.seconds; },
            [&](const Action::RescanAllFavourites) { favourite.rescanAll(scanner_obj, state.target_info.target_type); },
            // end of favourite stuff.

            [&](const Action::RestartScan) {
              hit.reset();
              state.fav_refresh_seconds = -1;
              state.hit_refresh_seconds = -1;
              state.target_info.target_type = TargetType::invalid;
              state.target_info.value = {};
            },
            [&](const Action::SetTargetInfo& a) {
              state.target_info.target_type = a.type;
              state.target_info.value = a.value;
              if (a.mask.has_value()) state.target_info.mask = a.mask;
            },
            [&](const Action::UndoScan) { hit.restore_old_hits(); },
            [&](const std::monostate&) {}},
        Pending);
  }
}

bool saveTenken(std::filesystem::path save_path, const std::vector<FavouriteInfoT>& favourites) {
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

    std::filesystem::create_directories(save_path.parent_path());
    std::ofstream saveFile(save_path);
    saveFile << savedState.dump(2);

  } catch (...) {
    return false;
  }
  return true;
}

bool loadTenken(std::filesystem::path save_path, std::vector<FavouriteInfoT>& favourites) {
  try {
    json loadedState;

    std::ifstream loadFile(save_path);

    if (!loadFile) {
      Log::error("Failed to open save from path. Are you sure it exists?");
      return 1;
    }

    loadedState = json::parse(loadFile);

    if (loadedState.value("version", 0) != 1) {
      Log::error(
          "Expected version and current version do not match for save file. Are you on a newer/older version than "
          "when "
          "you saved? If not...Yeah IDK what happened something is wrong clearly. Here is the supposed version" +
          std::to_string(loadedState.value("version", 0)) +
          " . And the only reason this log file is so unnecessarily long is because I felt like it. Anyways good "
          "luck "
          "with this problem someone who is probably me.");
      return false;
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
    Log::error("Failed to load. idk why.");
    return false;
  }
  return true;
}
