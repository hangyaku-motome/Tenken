#pragma once

#include "imgui.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

enum class TargetTypeT {
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

enum class RelativeStatus { UNCHANGED, CHANGED, INCREASED, DECREASED, UNSET };

struct HitInfoT {
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  uint64_t location;
  std::vector<uint8_t> bytes_around;
  RelativeStatus Status = RelativeStatus::UNSET;
};

struct MapInfoT {
  uint64_t start;
  uint64_t end;
  std::string name;
};

struct WindowInfoT {
  float H = 0;
  float W = 0;
  int XPos = 0;
  int YPos = 0;
  ImGuiWindowFlags flags;
};

struct DisplayInfoT {
  int display_w = 0;
  int display_h = 0;

  float TopMenuHeight = 0;
};

// should we really be initalising defaults like this?
struct ProcessInfoT {
  int pid = 0;
  std::string FieldComm = "";
  std::string FieldCmdline = "";
  int uid = 0;
};

struct TargetInfoT {
  std::vector<uint8_t> value;
  TargetTypeT TargetType = TargetTypeT::Invalid;
};

// we can merge some of these types and just add a bool for "favourite or hit".
enum class OpType {
  NONE,
  INIT_SCANNER,
  FIRST_SCAN,
  EDIT,
  FILTER,
  ADD_TO_FAVOURITES,
  REMOVE_FROM_FAVOURITES,
  CHANGE_NAME, // Questionable entry.
  REFRESH,
  REFRESH_ALL,
  FREEZE,
  UNFREEZE
};

enum class DataType { INVALID, HIT, FAVOURITE };

// at some point we might want to remove the value members and just calculate
// from bytes. for now, this is fine. // Yeah, to add to this...If a byte is
// invalid, we can draw it as ##. We can have an  vector of bool for each byte
// for valid/invalid...Eh. Will need some thinking.
struct FavouriteInfoT {
  uint64_t location;
  std::vector<uint8_t> value;
  std::vector<uint8_t> previous_value;
  std::vector<uint8_t> bytes_around;
  std::vector<uint8_t>
      previous_bytes_around; // thought about adding to hits as well but meh.
  RelativeStatus Status = RelativeStatus::UNSET;
  bool Frozen = false;
  std::string Description = "";
  std::vector<uint8_t> frozen_value;
};

struct Action {
  OpType Type = OpType::NONE;
  DataType WorkOn;
  std::optional<uint64_t> index;
  std::optional<std::vector<uint8_t>> newval;
  std::optional<std::string> newname;
  std::optional<RelativeStatus> KeepType;
  std::optional<bool> BasedOnCurrentValues =
      false; // we might wanna remove the default value.
};
