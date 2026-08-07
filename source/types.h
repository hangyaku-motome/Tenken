#pragma once

#include <array>
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

// some of these constexpr should be file local, not here, and arguably some classes/structs?

constexpr int32_t bytes_before = 32;
constexpr int32_t bytes_after = 32;

constexpr float epsilon = 0.1F;
constexpr char hex[] = "0123456789ABCDEF";
constexpr auto popup_flags =
    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar;

// fix invalid and unset not being first.
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

struct HitInfo {
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

struct MapInfo {
  std::string name;
  uint64_t start;
  uint64_t end;
  MapType type;

  bool operator<(const MapInfo& m) const { return start < m.start; };
};

struct ProcessInfo {
  std::string name;
  std::string cmdline;
  int pid = 0;
};

struct TargetInfo {
  std::vector<uint8_t> value{};
  std::optional<std::vector<bool>> mask;
  TargetType target_type = TargetType::invalid;
};

struct FavouriteInfo {
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  std::string desc;
  std::vector<uint8_t> bytes_around;
  std::vector<uint8_t> frozen_value;
  uint64_t location;
  float freeze_duration = -1;  // could merge frozen and freeze_duration but meh.
  TargetType type;
  RelativeStatus status = RelativeStatus::unset;
  bool frozen = false;
  bool is_ptr_backed = false;
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

struct Region {
  MappedRegion mapped_region;
  MapInfo map;
};

struct Snapshot {
  std::vector<Region> regions;
};

enum class ScanType { Nothing, Hit, HitFilter, HitRescan, Unknown, Pointer };

struct SessionState {
  TargetInfo target_info;
  ProcessInfo target_proc_info;
  std::atomic<ScanType> scan_type;
  std::atomic<float> scan_progress;
  float hit_refresh_seconds = -1;  // -1 disabled. 0 enabled icon. >= 0.3 active.
  float fav_refresh_seconds = -1;  // -1 disabled. 0 enabled icon. >= 0.3 active.
  std::vector<MapInfo> active_regions;
  std::atomic<bool> is_unknown_value_scan = false;
  Snapshot snapshots;
};

//
namespace Pointer {
constexpr uint32_t default_search_after = 2048;
constexpr uint32_t default_search_before = 0;
constexpr uint8_t default_scan_depth = 5;

struct ScanInfo {
  uint32_t search_after = default_search_after;
  uint32_t search_before = default_search_before;
  uint8_t scan_depth = default_scan_depth;
};

struct InitConfig {
  uint64_t address = 0;
  ScanInfo info;
};

constexpr int32_t size = 8;

constexpr uint64_t magic_bytes = 0xEE32BE81AAAAFEAF;
constexpr uint8_t file_version = 1;
constexpr uint8_t max_depth = 8;

// file layout:
// magic_bytes uint64_t
// file_version uint8_t
// entry_size sizeof(PointerChain) uint8_t
// entry_start_point uint64_t
//

// string table.
// each string entry is:
// uint8_t len, then rest of string.
// chain table.

// Important. Offsets are target -> root. Meaning, resolving or displaying them needs to reverse the valid offsets to
// work properly.

// this is how they are saved to file.
struct Chain {
  std::array<int64_t, Pointer::max_depth> offsets;
  uint64_t offset_in_module;
  uint32_t module_id;
  uint8_t valid_offsets;
};

// this is how they'll look when loaded in.
struct PrettyChain {
  std::vector<int64_t> offsets;
  std::string module_name;
  uint64_t offset_in_module;
};
};  // namespace Pointer

//
//
// Action stuff.

namespace Action {

struct TargetProcChosen {
  ProcessInfo chosen_proc;
};

struct FirstScan {
  TargetInfo target_info;
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
  std::vector<uint8_t> value;
  uint64_t index;
};

struct IsFreezeFavourite {
  uint64_t index;
  bool freeze;
};

struct FreezeValueFavourite {
  std::vector<uint8_t> value;
  uint64_t index;
};

struct DescFavourite {
  std::string value;
  uint64_t index;
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
  std::vector<uint8_t> value;
  std::optional<std::vector<bool>> mask;  // does this really need to be optional. maybe. will check later.
  TargetType type;
};

struct UndoScan {};

struct StartPointerScan {
  Pointer::InitConfig init_config;
};

struct ResolvePointerResult {uint64_t target_address;};

};  // namespace Action

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};

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
                                   Action::UndoScan,
                                   Action::StartPointerScan,
                                   Action::ResolvePointerResult>;

//
// End of Action stuff.
