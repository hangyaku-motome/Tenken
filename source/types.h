#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class TargetTypeT {
  Int8,
  Int16,
  Int32,
  Int64,
  Float,
  Double,
  String,
  Invalid
};

struct HitInfoT {
  std::vector<uint8_t> value;
  uint64_t location;
  std::vector<uint8_t> bytes_around;
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
};

struct DisplayInfoT {
  int display_w = 0;
  int display_h = 0;

  float TopMenuHeight = 0;

  WindowInfoT Hit;
  WindowInfoT Log;
  WindowInfoT Search;
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
  bool IsUnsigned = false;
};

struct ChosenParams {
  ProcessInfoT TargetProc;
  TargetInfoT TargetValInfo;
};

inline std::string TargetTypeToString(TargetTypeT TargetType) {
  switch (TargetType) {
  case TargetTypeT::Int8:
    return "Int8";
  case TargetTypeT::Int16:
    return "Int16";
  case TargetTypeT::Int32:
    return "Int32";
  case TargetTypeT::Int64:
    return "Int64";
  case TargetTypeT::Float:
    return "Float";
  case TargetTypeT::Double:
    return "Double";
  case TargetTypeT::String:
    return "String";
  default:
    return "Invalid";
  }
}

// the names might be a bit confusing..