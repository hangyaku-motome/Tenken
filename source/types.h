#pragma once

#include <imgui.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/mman.h>
#endif

struct ReadBlock {
  std::vector<uint8_t> read_bytes{};
  int64_t offset_from_adr = 0;
};

namespace Context {

static constexpr uint8_t BytesBefore = 32;
static constexpr uint8_t BytesAfter = 32;

};  // namespace Context

constexpr auto DefaultPopupFlags =
    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar;

// TODO:fix invalid and unset not being first.
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
  ReadBlock bytes_around;
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  uint64_t location;
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
  std::optional<std::vector<bool>> mask;  // does this reaaaaly have to be optional?
  TargetType target_type = TargetType::invalid;
};

struct FavouriteInfo {
  ReadBlock bytes_around;
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  std::vector<uint8_t> frozen_value;
  std::string desc;
  uint64_t location;
  float freeze_duration = -1;  // TODO: should merge frozen and frreeze duration
  TargetType type;
  RelativeStatus status = RelativeStatus::unset;
  bool frozen = false;
  bool is_ptr_backed = false;
};

struct Region {
  MapInfo map;
  char* ptr = nullptr;

  uint64_t size() const { return map.end - map.start; };

  Region() = default;

  Region(const MapInfo& m, char* p)
      : map(m),
        ptr(p) {}

  ~Region() {
    if (ptr) {
#ifdef _WIN32
      VirtualFree(ptr, 0, MEM_RELEASE);
#else
      munmap(ptr, map.end - map.start);
#endif
    }
  };

  Region(const Region&) = delete;
  Region& operator=(const Region&) = delete;

  Region(Region&& r) noexcept
      : map(r.map),
        ptr(r.ptr) {
    r.ptr = nullptr;
  };

  Region& operator=(Region&& r) noexcept {
    if (ptr) {
#ifdef _WIN32
      VirtualFree(ptr, 0, MEM_RELEASE);
#else
      munmap(ptr, map.end - map.start);
#endif
    }
    ptr = r.ptr;
    map = r.map;
    r.ptr = nullptr;
    return *this;
  };
};

struct Snapshot {
  std::vector<Region> regions;
};

enum class ScanType { Nothing, Hit, HitFilter, HitRescan, Unknown, Pointer };

// it doessss kind of feeel bloated and I could probably make this smaller...maybe.
struct SessionState {
  TargetInfo target_info;
  ProcessInfo target_proc_info;
  std::vector<MapInfo> active_regions;
  Snapshot snapshot;
  std::atomic<ScanType> scan_type;
  std::atomic<float> scan_progress;
  float hit_refresh_seconds = -1;  // -1 disabled. 0 enabled icon. >= 0.3 active.
  float fav_refresh_seconds = -1;  // -1 disabled. 0 enabled icon. >= 0.3 active.
};

// should I reaaally be setting defaults?
namespace Pointer {
constexpr uint32_t DefaultSearchAfter = 2048;
constexpr uint32_t DefaultSearchBefore = 0;
constexpr uint8_t DefaultScanDepth = 5;
constexpr uint64_t MagicBytes = 0xEE32BE81AAAAFEAF;
constexpr uint8_t MaxDepth = 8;

struct ScanInfo {
  uint32_t search_after = DefaultSearchAfter;
  uint32_t search_before = DefaultSearchBefore;
  uint8_t scan_depth = DefaultScanDepth;
};

struct InitConfig {
  uint64_t address = 0;
  ScanInfo info;
};

// this is how they are saved to file.
struct Chain {
  std::array<int64_t, Pointer::MaxDepth> offsets;
  uint64_t offset_in_module;
  uint32_t module_id;  // does this really have to be an uint32_t ?
  uint8_t valid_offsets;
};

constexpr uint8_t ChainSize{sizeof(Pointer::Chain)};

// miiiiiight as well

static_assert(offsetof(Chain, offsets) == 0);
static_assert(offsetof(Chain, offset_in_module) == 64);
static_assert(offsetof(Chain, module_id) == 72);
static_assert(offsetof(Chain, valid_offsets) == 76);
static_assert(sizeof(Chain) == 80);

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

namespace Scan {

struct StartUnknownValue {};

struct StartNormal {
  TargetInfo target_info;
};

struct Restart {};

struct Undo {};

struct StartPointer {
  Pointer::InitConfig init_config;
};

}  // namespace Scan

namespace Hit {

struct Write {
  std::vector<uint8_t> value;
  uint64_t index;
};

struct Rescan {
  uint64_t index;
};

struct RescanAll {};

struct RegularRefresh {
  float seconds;
};
}  // namespace Hit

namespace Favourite {
struct AddHit {
  uint64_t index;
};

struct Remove {
  uint64_t index;
};

struct Write {
  std::vector<uint8_t> value;
  uint64_t index;
};

struct IsFreeze {
  uint64_t index;
  bool freeze;
};

struct FreezeValue {
  std::vector<uint8_t> value;
  uint64_t index;
};

struct Desc {
  std::string value;
  uint64_t index;
};

struct Rescan {
  uint64_t index;
};

struct RegularRefresh {
  float seconds;
};

struct RescanAll {};
}  // namespace Favourite

struct TargetProcChosen {
  ProcessInfo chosen_proc;
};

struct FilterByValue {
  std::vector<uint8_t> value;
};

struct filterByStatus {
  RelativeStatus status;
};

struct SetTargetInfo {
  std::vector<uint8_t> value;
  std::optional<std::vector<bool>> mask;  // TODO:does this really need to be optional. maybe. will check later.
  TargetType type;
};

struct ResolvePointerResult {
  std::filesystem::path save_path;
  uint64_t target_address;
};

struct SaveTenken {
  std::filesystem::path path;
};

struct LoadTenken {
  std::filesystem::path path;
};

};  // namespace Action

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};

using PendingAction = std::variant<std::monostate,
                                   Action::TargetProcChosen,
                                   Action::Scan::StartNormal,
                                   Action::Scan::StartUnknownValue,
                                   Action::Scan::Undo,
                                   Action::Scan::StartPointer,
                                   Action::Scan::Restart,
                                   Action::Hit::Write,
                                   Action::Hit::Rescan,
                                   Action::Hit::RescanAll,
                                   Action::Hit::RegularRefresh,
                                   Action::Favourite::AddHit,
                                   Action::Favourite::Remove,
                                   Action::Favourite::Write,
                                   Action::Favourite::FreezeValue,
                                   Action::Favourite::IsFreeze,
                                   Action::Favourite::Rescan,
                                   Action::Favourite::RescanAll,
                                   Action::Favourite::Desc,
                                   Action::Favourite::RegularRefresh,
                                   Action::SetTargetInfo,
                                   Action::ResolvePointerResult,
                                   Action::FilterByValue,
                                   Action::filterByStatus,
                                   Action::SaveTenken,
                                   Action::LoadTenken>;

//
// End of Action stuff.
