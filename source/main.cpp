#include <imgui-filebrowser/imfilebrowser.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <X11/Xdefs.h>

#include <chrono>
#include <filesystem>
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
#include "PointerList.h"
#include "PointerW.h"
#include "scan_ops.h"
#include "Scanner.h"
#include "SearchW.h"
#include "TargetPopUp.h"
#include "types.h"
#include "utils.h"

using nlohmann::json;

constexpr auto file_browser_flags = ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CloseOnEsc |
                                    ImGuiFileBrowserFlags_CreateNewDir | ImGuiFileBrowserFlags_EditPathString;

// ptr scan save:
// on linux, .local/state/Tenken/Pointer/{name}/scan-{date}
// on windows, I don't know. I forgot. I need to look at that again.

void resolveActions(Scanner& scanner_obj,
                    const std::vector<PendingAction>& actions,
                    SessionState& state,
                    std::thread& scanner_thread,
                    HitList& hit,
                    FavouriteList& favourite,
                    PointerList& pointer

);

bool saveTenken(const std::filesystem::path& save_path, const std::vector<FavouriteInfo>& favourites);
bool loadTenken(const std::filesystem::path& load_path, std::vector<FavouriteInfo>& favourites);

// bool savePtrScanResult(const std::vector<PointerChain>& chains, const std::string& exec_name);

int main() {
  if (Platform::checkPermission() == false) {
    printf("Please give the necessary permissions to run this program. Consult the README for details.\n");
    return 1;
  }

  // Start up Dear ImGui.
  std::string imgui_init_path_str = Platform::getImguiInitPath();
  GLFWwindow* window = initaliseImgui(imgui_init_path_str);
  ImGuiIO& io = ImGui::GetIO();
  ImVec4 clear_color = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);

  ImGui::FileBrowser save_dialog(file_browser_flags);
  ImGui::FileBrowser load_dialog(file_browser_flags);

  // Start up Tenken.
  SessionState state;

  Scanner scanner;
  std::thread scanner_thread;

  HitList hit_l;
  FavouriteList favourite_l;
  PointerList pointer_l;

  SearchW search_w;
  HitW hit_w;
  FavouriteW favourite_w;
  LogW log_w;
  PointerW pointer_w;
  HexW hex_w(scanner);
  DataInspectorW data_inspector_w(scanner);

  TargetPopUp target_popup;
  MapsPopUp map_popup(scanner);

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
    setDefaultDisplay();

    std::vector<PendingAction> actions;

    std::string menu_bar_action = mainMenuBarCycle(save_dialog,
                                                   load_dialog,
                                                   target_popup.clicked_,
                                                   map_popup.clicked_,
                                                   log_w.enabled_,
                                                   hex_w.enabled_,
                                                   data_inspector_w.enabled_,
                                                   pointer_w.enabled_);

    if (save_dialog.HasSelected()) {
      saveTenken(save_dialog.GetSelected(), favourite_l.getAll());
      save_dialog.Close();
    };
    if (load_dialog.HasSelected()) {
      std::vector<FavouriteInfo> new_favourites;
      if (loadTenken(load_dialog.GetSelected(), new_favourites)) favourite_l.assignNew(new_favourites);
      load_dialog.Close();
    }

    // Target popup.
    actions.push_back(target_popup.cyclePopUp());

    // Region popup.
    map_popup.cyclePopUp(state.active_regions);

    // Hits window.
    if (hit_l.try_lock()) {
      actions.push_back(hit_w.cycleW(hit_l.getRef(), state));
      hit_l.unlock();
    } else
      actions.push_back(hit_w.cycleW({}, state));

    // Search window.
    actions.push_back(search_w.cycleW(state.target_info, state.target_proc_info.pid));

    // Favourite window.
    if (favourite_l.try_lock()) {
      actions.push_back(favourite_w.cycleW(favourite_l.getRef(), state));
      favourite_l.unlock();
    } else
      actions.push_back(favourite_w.cycleW({}, state));

    // Log window.
    log_w.cycleW();

    // Hex window.
    hex_w.cycleW();

    // Data Inspector window.
    data_inspector_w.cycleW();

    // Pointer window.
    actions.push_back(pointer_w.cycleW(state, pointer_l));

    // dialogs.
    save_dialog.Display();
    load_dialog.Display();

    // ui stuff is over.
    endFrame(static_cast<int32_t>(io.DisplaySize.x), static_cast<int32_t>(io.DisplaySize.y), clear_color, window);

    // resolve actions.
    resolveActions(scanner, actions, state, scanner_thread, hit_l, favourite_l, pointer_l);
    actions.clear();

    // regular refresh fav
    if (state.fav_refresh_seconds >= 0.3) {
      auto favouriterefresh = std::chrono::duration_cast<std::chrono::milliseconds>(loop_time - favourite_refresh_time);
      if (favouriterefresh.count() >= static_cast<int64_t>(state.fav_refresh_seconds * 1000)) {
        favourite_l.rescanAll(scanner);
        favourite_refresh_time = loop_time;
      }
    }

    // regular refresh hit
    if (state.hit_refresh_seconds >= 0.3) {
      auto hitrefresh = std::chrono::duration_cast<std::chrono::milliseconds>(loop_time - hit_refresh_time);
      if (hitrefresh.count() >= static_cast<int64_t>(state.hit_refresh_seconds * 1000)) {
        ScanOp::runOnScannerThread(scanner_thread, state, ScanType::HitRescan, [&]() {
          ScanOp::rescanAllHits(scanner, hit_l, state.scan_progress, state.target_info.target_type);
        });
        hit_refresh_time = loop_time;
      }
    }
  }
  exitImgui(window);
  favourite_l.endFreezeThread();
  if (scanner_thread.joinable()) scanner_thread.join();
  return 0;
}

void resolveActions(Scanner& scanner,
                    const std::vector<PendingAction>& actions,
                    SessionState& state,
                    std::thread& scanner_thread,
                    HitList& hit,
                    FavouriteList& favourite,
                    PointerList& pointer

) {
  for (auto& Pending : actions) {
    std::visit(
        overloaded{[&](const Action::TargetProcChosen& a) {
                     hit.reset();
                     favourite.reset();
                     scanner.init(a.chosen_proc.pid);
                     state.target_proc_info = a.chosen_proc;
                     state.active_regions = {};
                     favourite.startFreezeThread(scanner);
                   },
                   [&](const Action::FirstScan) {
                     ScanOp::runOnScannerThread(scanner_thread, state, ScanType::Hit, [&]() {
                       auto hits =
                           ScanOp::startScan(scanner, state.target_info, state.scan_progress, state.active_regions);
                       hit.assignNew(std::move(hits));
                     });
                   },
                   [&](const Action::StartUnknownValueScan) {
                     state.is_unknown_value_scan =
                         true;  // a way to know if we are doing it actively. it won't need Snapshot after scan 1.
                     ScanOp::runOnScannerThread(scanner_thread, state, ScanType::Unknown, [&]() {
                       state.snapshots = scanner.getSnapshot(state.active_regions, state.scan_progress);
                     });
                   },
                   [&](const Action::FilterByValue& a) {
                     ScanOp::runOnScannerThread(scanner_thread, state, ScanType::HitFilter, [&, value = a.value]() {
                       ScanOp::rescanAllHits(scanner, hit, state.scan_progress, state.target_info.target_type);
                       hit.filter(value);
                     });
                   },
                   [&](const Action::filterByStatus& a) {
                     if (state.is_unknown_value_scan) {
                       ScanOp::runOnScannerThread(scanner_thread, state, ScanType::HitFilter, [&, status = a.status]() {
                         hit.assignNew(scanner.filterSnapshot(state.snapshots, status, state.target_info.target_type));
                         state.is_unknown_value_scan = false;
                         state.snapshots = {};
                       });
                     } else {
                       ScanOp::runOnScannerThread(scanner_thread, state, ScanType::HitFilter, [&, status = a.status]() {
                         ScanOp::rescanAllHits(scanner, hit, state.scan_progress, state.target_info.target_type);
                         hit.filter(status);
                       });
                     }
                   },
                   [&](const Action::WriteHit& a) {
                     hit.write(scanner, a.index, a.value);
                     hit.rescan(scanner, a.index, state.target_info.target_type);
                   },
                   [&](const Action::RescanHit& a) { hit.rescan(scanner, a.index, state.target_info.target_type); },
                   [&](const Action::RescanAllHits) {
                     ScanOp::runOnScannerThread(scanner_thread, state, ScanType::Hit, [&]() {
                       ScanOp::rescanAllHits(scanner, hit, state.scan_progress, state.target_info.target_type);
                     });
                   },
                   [&](const Action::RegularRefreshHits& a) { state.hit_refresh_seconds = a.seconds; },

                   // start of favourite stuff.
                   [&](const Action::AddFavourite& a) {
                     favourite.add(hit.getIndex(a.hitIndex), state.target_info.target_type);
                   },
                   [&](const Action::RemoveFavourite& a) { favourite.remove(a.index); },
                   [&](const Action::WriteFavourite& a) {
                     favourite.write(scanner, a.index, a.value);
                     favourite.rescan(scanner, a.index);
                   },
                   [&](const Action::IsFreezeFavourite& a) { favourite.setFreeze(a.index, a.freeze); },
                   [&](const Action::FreezeValueFavourite& a) { favourite.setFreezeVal(a.index, a.value); },
                   [&](const Action::DescFavourite& a) { favourite.setDesc(a.index, a.value); },
                   [&](const Action::RescanFavourite& a) { favourite.rescan(scanner, a.index); },
                   [&](const Action::RegularRefreshFavourite& a) { state.fav_refresh_seconds = a.seconds; },
                   [&](const Action::RescanAllFavourites) { favourite.rescanAll(scanner); },
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
                   [&](const Action::StartPointerScan& a) {
                     ScanOp::runOnScannerThread(scanner_thread, state, ScanType::Pointer, [&, info = a.init_config]() {
                       auto path = scanner.findPointerCandidates(
                           scanner.getSnapshot(state.active_regions, state.scan_progress), info);
                       pointer.open_file(path);
                     });
                   },
                   [&](const std::monostate&) {}},
        Pending);
  }
}

bool saveTenken(const std::filesystem::path& save_path, const std::vector<FavouriteInfo>& favourites) {
  try {
    json save_state;
    save_state["version"] = 1;
    save_state["favourites"] = json::array();
    for (const auto& favourite : favourites) {
      json item;

      std::stringstream location_stream;

      location_stream << std::hex << std::showbase << favourite.location;
      item["location"] = location_stream.str();
      item["value"] = favourite.value;  // raw bytes for now. should be fine.
      item["desc"] = favourite.desc;
      item["type"] = targetTypeToStr(favourite.type);

      save_state["favourites"].push_back(item);
    }

    std::filesystem::create_directories(save_path.parent_path());
    std::ofstream save_stream(save_path);
    save_stream << save_state.dump(2);

  } catch (...) {
    Log::error("Failed to save. Noo idea why.");
    return false;
  }
  Log::info("Succesfully saved save (...should have)");
  return true;
}

bool loadTenken(const std::filesystem::path& load_path, std::vector<FavouriteInfo>& favourites) {
  try {
    json loaded_state;

    std::ifstream load_file(load_path);

    if (!load_file) {
      Log::error("Failed to open save from path. Are you sure it exists?");
      return false;
    }

    loaded_state = json::parse(load_file);

    if (loaded_state.value("version", 0) != 1) {
      Log::error(
          "Expected version and current version do not match for save file. Are you on a newer/older version than "
          "when "
          "you saved? If not...Yeah IDK what happened something is wrong clearly. Here is the supposed version" +
          std::to_string(loaded_state.value("version", 0)) +
          " . And the only reason this log file is so unnecessarily long is because I felt like it. Anyways good "
          "luck "
          "with this problem someone who is probably me.");
      return false;
    }
    favourites.clear();
    for (const auto& item : loaded_state.at("favourites")) {
      FavouriteInfo favourite;

      favourite.desc = item.at("desc").get<std::string>();
      favourite.location = std::stoull(item.at("location").get<std::string>(), nullptr, 16);

      std::vector<uint8_t> value;
      for (const auto& byte : item.at("value")) {
        value.push_back(byte);
      }
      favourite.value = value;
      favourite.type = strToTargetType(item["type"].get<std::string>());

      favourites.push_back(favourite);
    }
  } catch (...) {
    Log::error("Failed to load. idk why.");
    return false;
  }
  Log::info("Succesfully loaded save (...should have)");
  return true;
}

// well this is just kind of outdated and useless now. I'll keep it for a little bit longer though.
// bool savePtrScanResult(const std::vector<Pointer::Chain>& chains, const std::string& exec_name) {
//  try {
//   auto date = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

// std::string date_str = std::format("{:%Y-%m-%d_%H:%M:%S}", date);

// date_str += ".json";

// std::filesystem::path save_path = Platform::getTenkenStatePath() / "Pointer" / exec_name / date_str;
// std::filesystem::create_directories(save_path.parent_path());

// json save_state;
// save_state["version"] = 1;
// save_state["Pointers"] = json::array();

// for (const auto& chain : chains) {
//  json item;

// item["offset in module"] = chain.offset_in_module;
// item["offsets"] = json::array();
// for (const auto& offset : chain.offsets) {
//  item["offsets"].push_back(offset);
// }
// save_state["Pointers"].push_back(item);
// }
//
//   std::ofstream save_stream(save_path);
//  save_stream << save_state.dump(2);

// } catch (...) {
//  Log::error("couldn't save pointer to disk. Idk why.");
// return false;
// }
//
// Log::info("Saved chains to disk.");
// return true;
//}
