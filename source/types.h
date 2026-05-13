#pragma once

#include "imgui.h"
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

enum class TargetTypeT : std::int8_t {
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
  Invalid
};

enum class RelativeStatus : std::int8_t {
  UNCHANGED,
  CHANGED,
  INCREASED,
  DECREASED,
  UNSET
};

constexpr int8_t BYTES_BEFORE = 32;
constexpr int8_t BYTES_AFTER = 32;

struct HitInfoT {
  uint64_t location;
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  std::vector<uint8_t> bytes_around;
  RelativeStatus Status = RelativeStatus::UNSET;
};

struct MapInfoT {
  uint64_t start;
  uint64_t end;
  std::string name;
};

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
  std::string FieldComm;
  std::string FieldCmdline;
  int uid = 0;
};

struct TargetInfoT {
  std::vector<uint8_t> value{0, 0};
  TargetTypeT TargetType = TargetTypeT::Invalid;
};

enum class OpType : std::int8_t {
  NONE,
  INIT_SCANNER,
  FIRST_SCAN,
  EDIT,
  FILTER,
  ADD_TO_FAVOURITES,
  REMOVE_FROM_FAVOURITES,
  REFRESH,
  REGULAR_REFRESH,
  FREEZE,
  UNFREEZE,
  RESTART_STATE
};

// at some point we might want to remove the value members and just calculate
// from bytes. for now, this is fine. // Yeah, to add to this...If a byte is
// invalid, we can draw it as ##. We can have an  vector of bool for each byte
// for valid/invalid...Eh. Will need some thinking.
struct FavouriteInfoT {
  uint64_t location;
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  TargetTypeT TargetType;
  std::vector<uint8_t> bytes_around;
  std::vector<uint8_t>
      previous_bytes_around; // thought about adding to hits as well but meh.
  RelativeStatus Status = RelativeStatus::UNSET;
  bool Frozen = false;
  std::string Description;
  std::vector<uint8_t> frozen_value;
  float auto_refresh_seconds = -1;
  std::chrono::steady_clock::time_point since_last_auto_refresh;
};

struct HitWAction {
  OpType Type = OpType::NONE;
  std::optional<uint64_t> index;
  std::optional<std::vector<uint8_t>> buf;
  std::optional<float> seconds;
  std::optional<RelativeStatus> KeepType = RelativeStatus::UNSET;
};

struct FavouriteWAction {
  OpType Type = OpType::NONE;
  std::optional<uint64_t> index;
  std::optional<std::vector<uint8_t>> buf;
  std::optional<std::string> newname;
  std::optional<TargetTypeT> TargetType;
  // Refresh seconds and interpertation:
  // -1 means stop regular refreshing.
  // 0 means start regular refreshing.
  // 0.3 < x < 3.0 is valid.
  std::optional<float> seconds;
};

struct SearchWAction {
  OpType Type = OpType::NONE;
  bool BasedOnCurrentValues = false;
  std::optional<RelativeStatus> KeepType;
};

struct PendingAction {
  OpType OverrideType = OpType::NONE;
  HitWAction HitW;
  FavouriteWAction FavouriteW;
  SearchWAction SearchW;
};