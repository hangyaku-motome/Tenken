#pragma once
#include "types.h"
#include <stdexcept>

RelativeStatus tagChange(const char *old_bytes, const char *new_bytes,
                         TargetTypeT TargetType, uint64_t compare_size = 0);
std::vector<uint8_t> findBytesAround(uint32_t offset,
                                     const std::vector<uint8_t> &data,
                                     uint32_t size);

template <typename T>
std::vector<uint64_t> searchValue(const std::vector<uint8_t> &Data, T Target,
                                  float epsilon = 0.1F);
std::vector<uint64_t> searchRawValue(
    const std::vector<uint8_t> &Data, const std::vector<uint8_t> &TargetData,
    const std::vector<bool> &validBytes = {}); // byte scanning later.

std::string TargetTypeToStr(TargetTypeT TargetType);
std::string DataToStr(const std::vector<uint8_t> &Bytes,
                      TargetTypeT TargetType);
std::string RelativeStatusToStr(RelativeStatus Status);

template <typename T> T readAs(const std::vector<uint8_t> &buffer);

template <typename Func> void dispatchType(TargetTypeT type, Func &&func) {
  switch (type) {
  case TargetTypeT::uInt8:
    return func.template operator()<uint8_t>();
  case TargetTypeT::uInt16:
    return func.template operator()<uint16_t>();
  case TargetTypeT::uInt32:
    return func.template operator()<uint32_t>();
  case TargetTypeT::uInt64:
    return func.template operator()<uint64_t>();
  case TargetTypeT::Int8:
    return func.template operator()<int8_t>();
  case TargetTypeT::Int16:
    return func.template operator()<int16_t>();
  case TargetTypeT::Int32:
    return func.template operator()<int32_t>();
  case TargetTypeT::Int64:
    return func.template operator()<int64_t>();
  case TargetTypeT::Float:
    return func.template operator()<float>();
  case TargetTypeT::Double:
    return func.template operator()<double>();
  case TargetTypeT::String:
    return func.template operator()<std::string>();
  default:
    throw std::runtime_error("invalid type");
  }
}

template <typename T>
RelativeStatus compareValues(const char *old_bytes, const char *new_bytes,
                             float epsilon = 0.1F);