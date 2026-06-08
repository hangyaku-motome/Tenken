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

constexpr int8_t BYTES_BEFORE = 32;
constexpr int8_t BYTES_AFTER = 32;

constexpr float EPSILON = 0.1F;

constexpr char hex[] = "0123456789ABCDEF";

enum class TargetTypeT : int8_t {
  uInt8,
  uInt16,
  uInt32,
  uInt64,
  Int8,
  Int16,
  Int32,
  Int64,
  Float,
  Double,
  String,
  AOB,
  Invalid
};

enum class RelativeStatus : int8_t { UNCHANGED, CHANGED, INCREASED, DECREASED, UNSET };

struct HitInfoT {
  uint64_t location;
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  std::vector<uint8_t> bytes_around;
  RelativeStatus status = RelativeStatus::UNSET;
};

struct MapInfoT {
  uint64_t start;
  uint64_t end;
  std::string name;
};

// these next two might be unncessesary atp.
struct WindowInfoT {
  float H;
  float W;
  int XPos;
  int YPos;
  ImGuiWindowFlags flags;
};

struct DisplayInfoT {
  int display_w = 0;
  int display_h = 0;
  float TopMenuHeight = 0;
};

struct ProcessInfoT {
  int pid = 0;
  std::string name;
  std::string cmdline;
};

struct TargetInfoT {
  std::vector<uint8_t> value{};
  TargetTypeT TargetType = TargetTypeT::Invalid;
  std::optional<std::vector<bool>> mask;
};

struct FavouriteInfoT {
  uint64_t location;
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  std::string desc;
  RelativeStatus status = RelativeStatus::UNSET;
  std::vector<uint8_t> bytes_around;
  TargetTypeT type;

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
  TargetInfoT TargetInfo;
  ProcessInfoT TargetProcInfo;
  bool TargetChosen = false;
  std::atomic<bool> IsScanning = false;
  std::atomic<float> ScanProgress;
  float hitRefreshSeconds = -1;  // -1 disabled. 0 enabled icon. >= 0.3 active.
  float favRefreshSeconds = -1;  // -1 disabled. 0 enabled icon. >= 0.3 active.
  enum class SearchWStatusT : int8_t {
    DISABLED,
    FIRST,
    SECOND,
  } SearchWStatus = SessionState::SearchWStatusT::DISABLED;

  std::atomic<bool> IsUnknownnValueScan = false;
  Snapshot Snapshots;
};

//
// Action stuff.

namespace Action {

struct TargetProcChosen {
  ProcessInfoT chosenProc;
};

struct firstScan {
  TargetInfoT targetInfo;
};

struct startUnknownValueScan {};

struct filterByValue {
  std::vector<uint8_t> value;
};

struct filterByStatus {
  RelativeStatus status;
};

struct writeHit {
  uint64_t index;
  std::vector<uint8_t> value;
};

struct rescanHit {
  uint64_t index;
};

struct rescanAllHits {};

struct regularRefreshHits {
  float seconds;
};

// Favourite stuff.

struct addFavourite {
  uint64_t hitIndex;
};

struct removeFavourite {
  uint64_t index;
};

struct writeFavourite {
  uint64_t index;
  std::vector<uint8_t> value;
};

struct isFreezeFavourite {
  uint64_t index;
  bool freeze;
};

struct freezeValueFavourite {
  uint64_t index;
  std::vector<uint8_t> value;
};

struct descFavourite {
  uint64_t index;
  std::string value;
};

struct rescanFavourite {
  uint64_t index;
};

struct regularRefreshFavourite {
  float seconds;
};

struct rescanAllFavourites {};

// end of favourite stuff.

struct restartScan {};

struct setTargetInfo {
  TargetTypeT type;
  std::vector<uint8_t> value;
  std::optional<std::vector<bool>> mask;
};

struct undoScan {};

};  // namespace Action

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using PendingAction = std::variant<std::monostate,
                                   Action::TargetProcChosen,
                                   Action::firstScan,
                                   Action::startUnknownValueScan,
                                   Action::filterByValue,
                                   Action::filterByStatus,
                                   Action::writeHit,
                                   Action::addFavourite,
                                   Action::removeFavourite,
                                   Action::writeFavourite,
                                   Action::freezeValueFavourite,
                                   Action::isFreezeFavourite,
                                   Action::descFavourite,
                                   Action::restartScan,
                                   Action::regularRefreshHits,
                                   Action::regularRefreshFavourite,
                                   Action::rescanHit,
                                   Action::rescanAllHits,
                                   Action::rescanFavourite,
                                   Action::rescanAllFavourites,
                                   Action::setTargetInfo,
                                   Action::undoScan>;

//
// End of Action stuff.
