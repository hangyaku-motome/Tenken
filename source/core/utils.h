#pragma once
#include "types.h"
#include <cstdint>
#include <stdexcept>

template <typename T> RelativeStatus tagChange(T new_value, T old_value);

std::vector<uint8_t> findBytesAround(uint32_t offset, const std::vector<uint8_t> &data, uint32_t size);

template <typename T>
std::vector<uint64_t> searchValue(const std::vector<uint8_t> &Data, const T &Target,
                                  const std::vector<bool> &mask = {});

std::string targetTypeToStr(TargetTypeT TargetType);
template <typename T> std::string dataToStr(const std::vector<uint8_t> &Bytes);
std::string relativeStatusToStr(RelativeStatus Status);

template <typename Func> auto dispatchType(TargetTypeT type, Func &&func) {
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
  case TargetTypeT::AOB:
    return func.template operator()<std::vector<uint8_t>>();
  default:
    throw std::runtime_error("invalid type");
  }
}

template <typename T> T dataToType(const std::vector<uint8_t> &data);

bool strToAOBInfo(std::vector<uint8_t> &bytes, std::vector<bool> &mask);

std::string hexToStr(const uint8_t byte);

std::string dataToMaskedStr(const std::vector<uint8_t> &bytes, const std::vector<bool> &mask);
