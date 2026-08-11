#pragma once
#include <cstdint>
#include <stdexcept>

#include "types.h"

template <typename T> RelativeStatus tagChange(T new_value, T old_value);

template <typename T>
std::vector<uint64_t>
searchValue(const std::vector<uint8_t>& Data, const T& Target, const std::vector<bool>& mask = {});

TargetType strToTargetType(const std::string& string);
template <typename T> std::string dataToStr(const std::vector<uint8_t>& Bytes);
bool strToAOBInfo(std::vector<uint8_t>& bytes, std::vector<bool>& mask);

std::string targetTypeToStr(TargetType targetType);
std::string relativeStatusToStr(RelativeStatus Status);
std::string hexToStr(uint8_t byte);
std::string dataToMaskedStr(const std::vector<uint8_t>& bytes, const std::vector<bool>& mask);
std::string mapTypeToStr(const MapType type);

template <typename T> T dataToType(const std::vector<uint8_t>& data);
template <typename T> std::vector<uint8_t> typeToData(const T& val);

template <typename Func> auto dispatchType(TargetType type, Func&& func) {
  switch (type) {
    case TargetType::uInt8:
      return func.template operator()<uint8_t>();
    case TargetType::uInt16:
      return func.template operator()<uint16_t>();
    case TargetType::uInt32:
      return func.template operator()<uint32_t>();
    case TargetType::uInt64:
      return func.template operator()<uint64_t>();
    case TargetType::int8:
      return func.template operator()<int8_t>();
    case TargetType::int16:
      return func.template operator()<int16_t>();
    case TargetType::int32:
      return func.template operator()<int32_t>();
    case TargetType::int64:
      return func.template operator()<int64_t>();
    case TargetType::f32:
      return func.template operator()<float>();
    case TargetType::f64:
      return func.template operator()<double>();
    case TargetType::string:
      return func.template operator()<std::string>();
    case TargetType::aob:
      return func.template operator()<std::vector<uint8_t>>();
    default:
      throw std::runtime_error("invalid type");
  }
}

int64_t signedDiff(uint64_t a, uint64_t b);

std::filesystem::path getLatestFile(const std::filesystem::path& dir_path);

std::vector<uint8_t>
findBytesAround(const std::vector<uint8_t>& data, uint64_t offset, uint64_t bytes_before, uint64_t bytes_after);
