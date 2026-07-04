#pragma once

#include <atomic>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "imgui.h"

#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/mman.h>
#endif

constexpr int8_t bytes_before = 32;
constexpr int8_t bytes_after = 32;
constexpr float epsilon = 0.1F;
constexpr char hex[] = "0123456789ABCDEF";
constexpr auto popup_flags =
    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar;

// shouldn't invalid be first, and unset be first for TargetTypeT and RelativeStatus respectively? Also...Naming
// convention problems.
enum class TargetType : int8_t {
  uInt8,
  uInt16,
  uInt32,
  uInt64,
  int8,
  int16,
  int32,
  int64,
  f32,
  f64,
  string,
  aob,
  invalid
};

enum class RelativeStatus : int8_t { unchanged, changed, increased, decreased, unset };

struct HitInfoT {
  uint64_t location;
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  std::vector<uint8_t> bytes_around;
  RelativeStatus status = RelativeStatus::unset;
};

enum class MapType : int8_t {
  unset,
  mainExecCode,
  mainExecData,
  mainExecConst,
  sharedLibCode,
  sharedLibData,
  sharedLibConst,
  heap,
  anon,
  stack,
  kernelPages,
  unreadable
};

struct MapInfoT {
  uint64_t start;
  uint64_t end;
  std::string name;
  MapType type;
};

struct ProcessInfoT {
  int pid = 0;
  std::string name;
  std::string cmdline;
};

struct TargetInfoT {
  std::vector<uint8_t> value{};
  TargetType target_type = TargetType::invalid;
  std::optional<std::vector<bool>> mask;
};

struct FavouriteInfoT {
  uint64_t location;
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  std::string desc;
  RelativeStatus status = RelativeStatus::unset;
  std::vector<uint8_t> bytes_around;
  TargetType type;

  bool frozen = false;
  std::vector<uint8_t> frozen_value;
  float freeze_duration = -1;  // could merge frozen and freeze_duration but meh.
};

struct MappedRegion {
  char* ptr = nullptr;
  uint64_t size = 0;

  MappedRegion() = default;

  MappedRegion(char* p, uint64_t s)
      : ptr(p),
        size(s) {}

  ~MappedRegion() {
    if (ptr)
#ifdef _WIN32
      VirtualFree(ptr, 0, MEM_RELEASE);
#else
      munmap(ptr, size);
#endif
  }

  MappedRegion(MappedRegion&& o) noexcept
      : ptr(o.ptr),
        size(o.size) {
    o.ptr = nullptr;
  }

  MappedRegion& operator=(MappedRegion&&) = delete;
  MappedRegion(const MappedRegion&) = delete;
};

struct Snapshot {
  std::vector<MappedRegion> regions;
  std::vector<MapInfoT> maps;
};

struct SessionState {
  TargetInfoT target_info;
  ProcessInfoT target_proc_info;
  std::atomic<bool> is_scanning = false;
  std::atomic<float> scan_progress;
  float hit_refresh_seconds = -1;  // -1 disabled. 0 enabled icon. >= 0.3 active.
  float fav_refresh_seconds = -1;  // -1 disabled. 0 enabled icon. >= 0.3 active.
  std::vector<MapInfoT> active_regions;
  std::atomic<bool> is_unknown_value_scan = false;
  Snapshot snapshots;
};

//
// Action stuff.

namespace Action {

struct TargetProcChosen {
  ProcessInfoT chosen_proc;
};

struct FirstScan {
  TargetInfoT target_info;
};

struct StartUnknownValueScan {};

struct FilterByValue {
  std::vector<uint8_t> value;
};

struct filterByStatus {
  RelativeStatus status;
};

struct WriteHit {
  uint64_t index;
  std::vector<uint8_t> value;
};

struct RescanHit {
  uint64_t index;
};

struct RescanAllHits {};

struct RegularRefreshHits {
  float seconds;
};

// Favourite stuff.
// maybe put all related structs into a namespace?

struct AddFavourite {
  uint64_t hitIndex;
};

struct RemoveFavourite {
  uint64_t index;
};

struct WriteFavourite {
  uint64_t index;
  std::vector<uint8_t> value;
};

struct IsFreezeFavourite {
  uint64_t index;
  bool freeze;
};

struct FreezeValueFavourite {
  uint64_t index;
  std::vector<uint8_t> value;
};

struct DescFavourite {
  uint64_t index;
  std::string value;
};

struct RescanFavourite {
  uint64_t index;
};

struct RegularRefreshFavourite {
  float seconds;
};

struct RescanAllFavourites {};

// end of favourite stuff.

struct RestartScan {};

struct SetTargetInfo {
  TargetType type;
  std::vector<uint8_t> value;
  std::optional<std::vector<bool>> mask;
};

struct UndoScan {};

};  // namespace Action

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using PendingAction = std::variant<std::monostate,
                                   Action::TargetProcChosen,
                                   Action::FirstScan,
                                   Action::StartUnknownValueScan,
                                   Action::FilterByValue,
                                   Action::filterByStatus,
                                   Action::WriteHit,
                                   Action::AddFavourite,
                                   Action::RemoveFavourite,
                                   Action::WriteFavourite,
                                   Action::FreezeValueFavourite,
                                   Action::IsFreezeFavourite,
                                   Action::DescFavourite,
                                   Action::RestartScan,
                                   Action::RegularRefreshHits,
                                   Action::RegularRefreshFavourite,
                                   Action::RescanHit,
                                   Action::RescanAllHits,
                                   Action::RescanFavourite,
                                   Action::RescanAllFavourites,
                                   Action::SetTargetInfo,
                                   Action::UndoScan>;

//
// End of Action stuff.
