#include "utils.h"
#include "types.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

template <typename T> RelativeStatus tagChange(T new_value, T old_value) {
  if constexpr (std::is_same_v<T, std::string>) {
    if (old_value == new_value)
      return RelativeStatus::CHANGED;
    else
      return RelativeStatus::UNCHANGED;
  } else if constexpr (std::is_floating_point_v<T>) {
    if (std::isnan(old_value) || std::isnan(new_value))
      return RelativeStatus::CHANGED;

    float diff = old_value - new_value;
    if (std::abs(diff) <= EPSILON)
      return RelativeStatus::UNCHANGED;

    if (diff > 0)
      return RelativeStatus::INCREASED;

    return RelativeStatus::DECREASED;
  } else if constexpr (std::is_arithmetic_v<T>) {
    if (old_value == new_value)
      return RelativeStatus::UNCHANGED;
    if (new_value > old_value)
      return RelativeStatus::INCREASED;

    return RelativeStatus::DECREASED;
  }

  printf("how did we get here?\n");
  return RelativeStatus::UNSET;
}
template RelativeStatus tagChange<uint8_t>(uint8_t, uint8_t);
template RelativeStatus tagChange<uint16_t>(uint16_t, uint16_t);
template RelativeStatus tagChange<uint32_t>(uint32_t, uint32_t);
template RelativeStatus tagChange<uint64_t>(uint64_t, uint64_t);
template RelativeStatus tagChange<int8_t>(int8_t, int8_t);
template RelativeStatus tagChange<int16_t>(int16_t, int16_t);
template RelativeStatus tagChange<int32_t>(int32_t, int32_t);
template RelativeStatus tagChange<int64_t>(int64_t, int64_t);
template RelativeStatus tagChange<float>(float, float);
template RelativeStatus tagChange<double>(double, double);
template RelativeStatus tagChange<std::string>(std::string, std::string);
template RelativeStatus tagChange<std::vector<uint8_t>>(std::vector<uint8_t>, std::vector<uint8_t>);

//

std::vector<uint8_t> findBytesAround(const uint32_t offset, const std::vector<uint8_t> &data,
                                     const uint32_t size) {
  uint64_t START = offset < 32 ? 0 : offset - 32;
  uint64_t END = offset + 32 + size > data.size() ? data.size() : offset + 32 + size;

  std::vector<uint8_t> bytes(BYTES_BEFORE + BYTES_AFTER + size);
  memcpy(bytes.data(), &data[START], END - START);
  return bytes;
}

template <typename T>
std::vector<uint64_t> searchValue(const std::vector<uint8_t> &Data, T Target) {

  static_assert(std::is_arithmetic_v<T>, "SearchValue only works with numeric types");

  std::vector<uint64_t> FoundOffsets;

  T data_value;

  for (uint32_t i = 0; i + sizeof(T) <= Data.size(); i += sizeof(T)) {
    memcpy(&data_value, Data.data() + i, sizeof(T));
    if constexpr (std::is_floating_point_v<T>) {
      if (std::abs(data_value - Target) <= EPSILON)
        FoundOffsets.push_back(i);
    } else {
      if (data_value == Target)
        FoundOffsets.push_back(i);
    }
  }
  return FoundOffsets;
}
template std::vector<uint64_t> searchValue<uint8_t>(const std::vector<uint8_t> &, uint8_t);
template std::vector<uint64_t> searchValue<uint16_t>(const std::vector<uint8_t> &, uint16_t);
template std::vector<uint64_t> searchValue<uint32_t>(const std::vector<uint8_t> &, uint32_t);
template std::vector<uint64_t> searchValue<uint64_t>(const std::vector<uint8_t> &, uint64_t);
template std::vector<uint64_t> searchValue<int8_t>(const std::vector<uint8_t> &, int8_t);
template std::vector<uint64_t> searchValue<int16_t>(const std::vector<uint8_t> &, int16_t);
template std::vector<uint64_t> searchValue<int32_t>(const std::vector<uint8_t> &, int32_t);
template std::vector<uint64_t> searchValue<int64_t>(const std::vector<uint8_t> &, int64_t);
template std::vector<uint64_t> searchValue<float>(const std::vector<uint8_t> &, float);
template std::vector<uint64_t> searchValue<double>(const std::vector<uint8_t> &, double);

//

// I might wanna merge searchRawValue and searchValue but not right now.

std::vector<uint64_t>
searchRawValue(const std::vector<uint8_t> &Data, const std::vector<uint8_t> &TargetData,
               const std::vector<bool> &validBytes) { // validbytes for byte scanning later.

  std::vector<uint64_t> FoundOffsets;
  uint64_t TargetSize = TargetData.size();

  for (uint32_t i = 0; i + TargetSize <= Data.size(); ++i) {
    if (memcmp(&Data[i], TargetData.data(), TargetSize) == 0) {
      FoundOffsets.push_back(i);
    }
  }
  return FoundOffsets;
}

// tostr stuff

template <typename T> std::string dataToStr(const std::vector<uint8_t> &Bytes) {

  if (Bytes.empty())
    return "";

  if constexpr (std::is_same_v<T, std::string>) {
    return std::string(reinterpret_cast<const char *>(Bytes.data()), Bytes.size());
  } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
    return ""; // to be added.
  } else {
    T value;
    memcpy(&value, Bytes.data(), sizeof(T));
    return std::to_string(value);
  }
}
template std::string dataToStr<uint8_t>(const std::vector<uint8_t> &);
template std::string dataToStr<uint16_t>(const std::vector<uint8_t> &);
template std::string dataToStr<uint32_t>(const std::vector<uint8_t> &);
template std::string dataToStr<uint64_t>(const std::vector<uint8_t> &);
template std::string dataToStr<int8_t>(const std::vector<uint8_t> &);
template std::string dataToStr<int16_t>(const std::vector<uint8_t> &);
template std::string dataToStr<int32_t>(const std::vector<uint8_t> &);
template std::string dataToStr<int64_t>(const std::vector<uint8_t> &);
template std::string dataToStr<float>(const std::vector<uint8_t> &);
template std::string dataToStr<double>(const std::vector<uint8_t> &);
template std::string dataToStr<std::string>(const std::vector<uint8_t> &);
template std::string dataToStr<std::vector<uint8_t>>(const std::vector<uint8_t> &);

//

std::string targetTypeToStr(const TargetTypeT TargetType) {
  switch (TargetType) {
  case TargetTypeT::uInt8:
    return "uint8";
  case TargetTypeT::uInt16:
    return "uint16";
  case TargetTypeT::uInt32:
    return "uint32";
  case TargetTypeT::uInt64:
    return "uint64";
  case TargetTypeT::Int8:
    return "int8";
  case TargetTypeT::Int16:
    return "int16";
  case TargetTypeT::Int32:
    return "int32";
  case TargetTypeT::Int64:
    return "int64";
  case TargetTypeT::Float:
    return "float";
  case TargetTypeT::Double:
    return "double";
  case TargetTypeT::String:
    return "string";
  case TargetTypeT::Invalid:
  default:
    return "invalid";
  }
}

std::string relativeStatusToStr(const RelativeStatus Status) {
  switch (Status) {
  case RelativeStatus::INCREASED:
    return "Increased";
  case RelativeStatus::DECREASED:
    return "Decreased";
  case RelativeStatus::UNCHANGED:
    return "Unchanged";
  case RelativeStatus::CHANGED:
    return "Changed";
  case RelativeStatus::UNSET:
    return "Unset";
  }
  return "";
}

// end of tostr stuff.

// takes a typename T. takes data. casts it to that. returns that.
template <typename T> T datatoType(const std::vector<uint8_t> &data) {
  if constexpr (std::is_same_v<T, std::string>) {
    return std::string(reinterpret_cast<const char *>(data.data()), data.size());
  } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
    return data;
  } else {
    T returnVal;
    memcpy(&returnVal, data.data(), sizeof(T));
    return returnVal;
  }
}
template uint8_t datatoType<uint8_t>(const std::vector<uint8_t> &);
template uint16_t datatoType<uint16_t>(const std::vector<uint8_t> &);
template uint32_t datatoType<uint32_t>(const std::vector<uint8_t> &);
template uint64_t datatoType<uint64_t>(const std::vector<uint8_t> &);
template int8_t datatoType<int8_t>(const std::vector<uint8_t> &);
template int16_t datatoType<int16_t>(const std::vector<uint8_t> &);
template int32_t datatoType<int32_t>(const std::vector<uint8_t> &);
template int64_t datatoType<int64_t>(const std::vector<uint8_t> &);
template float datatoType<float>(const std::vector<uint8_t> &);
template double datatoType<double>(const std::vector<uint8_t> &);
template std::vector<uint8_t> datatoType<std::vector<uint8_t>>(const std::vector<uint8_t> &);
template std::string datatoType<std::string>(const std::vector<uint8_t> &);

//
