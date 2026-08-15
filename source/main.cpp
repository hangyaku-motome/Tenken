#include <imgui-filebrowser/imfilebrowser.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <X11/Xdefs.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include "DataInspectorW.h"
#include "display.h"
#include "FavouriteList.h"
#include "FavouriteW.h"
#include "HexW.h"
#include "HitList.h"
#include "HitW.h"
#include "Log.h"
#include "LogW.h"
#include "MainMenuBar.h"
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
#include "version.h"

using nlohmann::json;

void resolveActions(Scanner& scanner_obj,
                    const std::vector<PendingAction>& actions,
                    State& state,
                    std::thread& scanner_thread,
                    HitList& hit_l,
                    FavouriteList& favourite_l,
                    PointerList& pointer_l

);

bool saveTenken(const std::filesystem::path& save_path, const std::vector<FavouriteInfo>& favourites);
bool loadTenken(const std::filesystem::path& load_path, std::vector<FavouriteInfo>& favourites);

// TODO: maybeee for pointer an option to compare 2 pointer results and keep same ones, not just live process based
// validation?
// TODO: maybe a resetVolatile() for each window to reset buffers and temporary data? Whenever a window makes a finished
// request, it can reset itself with it.
// WARNING: pointer_l might go through a data race
int main() {
  if (Platform::checkPermission() == false) {
    printf("Please give the necessary permissions to run this program. Consult the README for details.\n");
    return 1;
  }

  // TODO: make sure directories are all created.

  // Start up Dear ImGui.
  std::string imgui_init_path_str = Platform::getImguiInitPath();
  GLFWwindow* window = initaliseImgui(imgui_init_path_str);
  ImGuiIO& io = ImGui::GetIO();
  ImVec4 clear_color = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);

  // Start up Tenken.
  State state;

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

  MainMenuBar menu_bar(target_popup.clicked_,
                       map_popup.clicked_,
                       log_w.enabled_,
                       hex_w.enabled_,
                       data_inspector_w.enabled_,
                       pointer_w.enabled_);

  auto hit_refresh_time = std::chrono::steady_clock::now();
  auto favourite_refresh_time = std::chrono::steady_clock::now();

  // Main loop.
  std::vector<PendingAction> actions;
  while (glfwWindowShouldClose(window) == 0) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }
    const auto loop_time = std::chrono::steady_clock::now();

    start_frame();
    setDefaultDisplay();

    // Menu bar
    actions.push_back(menu_bar.cycle(state.target_proc_info.name));

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
    // Okay frankly...I'm not sure how to feel about this whole "lock" thing we're doing with hit_l and favourite_l
    // but...Whatever.
    if (favourite_l.try_lock()) {
      actions.push_back(favourite_w.cycleW(favourite_l.getRef(), state));
      favourite_l.unlock();
    } else
      actions.push_back(favourite_w.cycleW({}, state));

    // Pointer window.
    actions.push_back(pointer_w.cycleW(state, pointer_l));

    // Log window.
    log_w.cycleW();

    // Hex window.
    hex_w.cycleW();

    // Data Inspector window.
    data_inspector_w.cycleW();

    // ui stuff is over.
    endFrame(static_cast<int32_t>(io.DisplaySize.x), static_cast<int32_t>(io.DisplaySize.y), clear_color, window);

    // resolve actions.
    resolveActions(scanner, actions, state, scanner_thread, hit_l, favourite_l, pointer_l);
    actions.clear();

    // regular refresh fav
    if (state.fav_refresh_seconds >= 0.3) {
      auto favourite_refresh =
          std::chrono::duration_cast<std::chrono::milliseconds>(loop_time - favourite_refresh_time);
      if (favourite_refresh.count() >= static_cast<int64_t>(state.fav_refresh_seconds * 1000)) {
        favourite_l.rescanAll(scanner);
        favourite_refresh_time = loop_time;
      }
    }

    // regular refresh hit
    if (state.hit_refresh_seconds >= 0.3) {
      auto hit_refresh = std::chrono::duration_cast<std::chrono::milliseconds>(loop_time - hit_refresh_time);
      if (hit_refresh.count() >= static_cast<int64_t>(state.hit_refresh_seconds * 1000)) {
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
                    State& state,
                    std::thread& scanner_thread,
                    HitList& hit_l,
                    FavouriteList& favourite_l,
                    PointerList& pointer_l) {
  for (auto& pending : actions) {
    std::visit(
        overloaded{
            [&](const Action::TargetProcChosen& a) {
              hit_l.reset();
              favourite_l.reset();
              scanner.init(a.chosen_proc.pid);
              state.target_proc_info = a.chosen_proc;
              state.active_regions = {};
              favourite_l.startFreezeThread(scanner);
            },
            [&](const Action::Scan::StartNormal) {
              ScanOp::runOnScannerThread(scanner_thread, state, ScanType::Hit, [&]() {
                auto hits = ScanOp::startScan(scanner, state.target_info, state.scan_progress, state.active_regions);
                hit_l.assignNew(std::move(hits));
              });
            },
            [&](const Action::Scan::StartUnknownValue) {
              state.scan_type = ScanType::Unknown;
              ScanOp::runOnScannerThread(scanner_thread, state, ScanType::Unknown, [&]() {
                state.snapshot = scanner.getSnapshot(state.active_regions, state.scan_progress);
              });
            },
            [&](const Action::FilterByValue& a) {
              ScanOp::runOnScannerThread(scanner_thread, state, ScanType::HitFilter, [&, value = a.value]() {
                ScanOp::rescanAllHits(scanner, hit_l, state.scan_progress, state.target_info.target_type);
                hit_l.filter(value);
              });
            },
            [&](const Action::filterByStatus& a) {
              if (state.scan_type == ScanType::Unknown) {
                ScanOp::runOnScannerThread(scanner_thread, state, ScanType::HitFilter, [&, status = a.status]() {
                  hit_l.assignNew(scanner.filterSnapshot(state.snapshot, status, state.target_info.target_type));
                  state.scan_type = ScanType::Nothing;
                  state.snapshot = {};
                });
              } else {
                ScanOp::runOnScannerThread(scanner_thread, state, ScanType::HitFilter, [&, status = a.status]() {
                  ScanOp::rescanAllHits(scanner, hit_l, state.scan_progress, state.target_info.target_type);
                  hit_l.filter(status);
                });
              }
            },
            [&](const Action::Hit::Write& a) {
              hit_l.write(scanner, a.index, a.value);
              hit_l.rescan(scanner, a.index, state.target_info.target_type);
            },
            [&](const Action::Hit::Rescan& a) { hit_l.rescan(scanner, a.index, state.target_info.target_type); },
            [&](const Action::Hit::RescanAll) {
              ScanOp::runOnScannerThread(scanner_thread, state, ScanType::Hit, [&]() {
                ScanOp::rescanAllHits(scanner, hit_l, state.scan_progress, state.target_info.target_type);
              });
            },
            [&](const Action::Hit::RegularRefresh& a) { state.hit_refresh_seconds = a.seconds; },

            // start of favourite stuff.
            [&](const Action::Favourite::AddHit& a) {
              favourite_l.add(hit_l.getIndex(a.index), state.target_info.target_type);
            },
            [&](const Action::Favourite::AddChain& a) {
              auto chain = pointer_l.get(a.index);
              auto region = scanner.getMapRegions();

              auto first = std::find_if(region.begin(), region.end(), [&](const auto& r) {
                return r.name == chain.module_name && r.type == MapType::mainExecData;
              });

              if (first == region.end()) {
                Log::error("I couldn't find the module this pointer was supposed to be in.......weird.");
                return;
              }
              if (std::find_if(region.begin(), region.end(), [&](const auto& r) {
                    return r.name == chain.module_name && r.type == MapType::mainExecData;
                  }) == region.end()) {
                Log::error(
                    "If you have seen this error message, PLEASE let me personally know. In conclusion, I cannot save "
                    "this pointer. But hey, if you tell me about this, I'll be more inclined to fix this!");
                return;
              }

              auto adr = scanner.resolveChain(chain, first->start);
              if (adr == 0) {
                Log::error("I couldn't resolve the pointer you just tried to save..");
                return;
              }

              favourite_l.add(chain, adr, a.target_type, a.target_size);
            },

            [&](const Action::Favourite::Remove& a) { favourite_l.remove(a.index); },
            [&](const Action::Favourite::Write& a) {
              favourite_l.write(scanner, a.index, a.value);
              favourite_l.rescan(scanner, a.index);
              favourite_l.setFreezeVal(a.index, a.value);
            },
            [&](const Action::Favourite::IsFreeze& a) { favourite_l.setFreeze(a.index, a.freeze); },
            [&](const Action::Favourite::FreezeValue& a) {
              favourite_l.setFreezeVal(a.index, a.value);
            },  // I think this is never actually used...
            [&](const Action::Favourite::Desc& a) { favourite_l.setDesc(a.index, a.value); },
            [&](const Action::Favourite::Rescan& a) { favourite_l.rescan(scanner, a.index); },
            [&](const Action::Favourite::RegularRefresh& a) { state.fav_refresh_seconds = a.seconds; },
            [&](const Action::Favourite::RescanAll) { favourite_l.rescanAll(scanner); },
            // end of favourite stuff.

            [&](const Action::Scan::Restart) {
              hit_l.reset();
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
            [&](const Action::Scan::Undo) { hit_l.restore_old_hits(); },
            [&](const Action::Scan::StartPointer& a) {
              ScanOp::runOnScannerThread(scanner_thread, state, ScanType::Pointer, [&, info = a.init_config]() {
                auto ok =
                    scanner.findInitialChains(scanner.getSnapshot(state.active_regions, state.scan_progress), info);
                if (ok)
                  pointer_l.openFile(
                      getLatestFile(Platform::getTenkenStatePath() / "Pointer" / state.target_proc_info.name));
              });
            },
            [&](const Action::ResolvePointerResult& a) {
              ScanOp::runOnScannerThread(scanner_thread, state, ScanType::Pointer, [&, info = a]() {
                pointer_l.openFile(info.save_path);

                scanner.resolveChainResult(pointer_l, a.target_address);

                pointer_l.openFile(getLatestFile(info.save_path.parent_path()));
              });
            },
            [&](const Action::SaveTenken& a) { saveTenken(a.path, favourite_l.getAll()); },
            [&](const Action::LoadTenken& a) {
              std::vector<FavouriteInfo> favourites;
              loadTenken(a.path, favourites);
              for (int32_t i = 0; i < favourites.size(); ++i) {
                if (!favourites[i].chain.has_value()) continue;

                auto region = scanner.getMapRegions();

                auto first = std::find_if(region.begin(), region.end(), [&](const auto& r) {
                  return r.name == favourites[i].chain->module_name && r.type == MapType::mainExecData;
                });

                if (first == region.end()) {
                  Log::error("I couldn't find the module favourite {} was supposed to be in..deleting.", i);
                  favourites.erase(favourites.begin() + i);
                  continue;
                }
                if (std::find_if(region.begin(), region.end(), [&](const auto& r) {
                      return r.name == favourites[i].chain->module_name && r.type == MapType::mainExecData;
                    }) == region.end()) {
                  Log::error(
                      "If you have seen this error message, let me personally know. In conclusion, I cannot "
                      "load these favourites. But hey, if you tell me about this, I'll be more inclined to fix this!");
                  return;
                }

                // TODO:Thinking of embedding region_start itself into Pointer::PrettyChain...
                auto adr = scanner.resolveChain(favourites[i].chain.value(), first->start);
                if (adr == 0) {
                  Log::error("It seems like I couldn't resolve favourite index {}, will not be adding it.", i);
                  favourites.erase(favourites.begin() + i);
                }

                favourites[i].location = adr;
              }
              favourite_l.assignNew(favourites);
            },
            [&](const std::monostate&) {}},
        pending);
  }
}

bool saveTenken(const std::filesystem::path& save_path, const std::vector<FavouriteInfo>& favourites) {
  json save_state;
  save_state["version"] = JsonSaveVersion;
  save_state["favourites"] = json::array();
  for (const auto& favourite : favourites) {
    json item;

    std::stringstream location_stream;

    location_stream << std::hex << std::showbase << favourite.location;
    item["value"] = favourite.value;  // raw bytes for now. should be fine.
    item["desc"] = favourite.desc;
    item["type"] = targetTypeToStr(favourite.type);

    if (favourite.chain.has_value()) {
      json chain;
      chain["offset in module"] = favourite.chain->offset_in_module;
      chain["module name"] = favourite.chain->module_name;
      chain["offsets"] = favourite.chain->offsets;

      item["chain"] = chain;
    } else
      item["location"] = location_stream.str();

    save_state["favourites"].push_back(item);
  }

  std::filesystem::create_directories(save_path.parent_path());
  std::ofstream save_stream(save_path.string() + ".json");
  save_stream << save_state.dump(2);

  if (!save_stream) {
    Log::error("Failed to save. Noo idea why.");
    return false;
  }
  Log::info("Succesfully saved save (...should have)");
  return true;
}

// this could probably just return favourites...
bool loadTenken(const std::filesystem::path& load_path, std::vector<FavouriteInfo>& favourites) {
  json loaded_state;

  std::ifstream load_stream(load_path);

  if (!load_stream) {
    Log::error("Failed to open save from path. Are you sure it exists?");
    return false;
  }

  loaded_state = json::parse(load_stream);

  if (loaded_state.value("version", 0) != JsonSaveVersion) {
    Log::error(
        "Expected version and current version do not match for save file. Are you on a newer/older version than when "
        "you saved? If not...Yeah IDK what happened something is wrong clearly. Here is the supposed version:{}. And "
        "the only reason this log file is so unnecessarily long is because I felt like it. Anyways good "
        "luck with this one.",
        loaded_state.value<int32_t>("version", 0));
    return false;
  }
  favourites.clear();
  for (const auto& item : loaded_state.at("favourites")) {
    FavouriteInfo favourite;

    favourite.desc = item.at("desc").get<std::string>();

    std::vector<uint8_t> value;
    for (const auto& byte : item.at("value")) {
      value.push_back(byte);
    }
    favourite.value = value;
    favourite.type = strToTargetType(item["type"].get<std::string>());

    if (item.contains("chain")) {
      Pointer::PrettyChain chain;
      chain.module_name = item["chain"]["module name"];
      chain.offset_in_module = item["chain"]["offset in module"];
      chain.offsets = item["chain"]["offsets"].get<std::vector<int64_t>>();
      favourite.chain = chain;
    } else
      favourite.location = std::stoull(item.at("location").get<std::string>(), nullptr, 16);

    favourites.push_back(favourite);
  }

  if (!load_stream) {
    Log::error("Failed to load. idk why.");
    return false;
  }
  Log::info("Succesfully loaded save (...should have)");
  return true;
}
