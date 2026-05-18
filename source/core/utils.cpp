#include "utils.h"
#include "LogW.h"
#include "types.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <type_traits>

RelativeStatus tagChange(const char *old_bytes, const char *new_bytes,
                         const TargetTypeT TargetType, uint64_t compare_size) {
  if (TargetType == TargetTypeT::Invalid) {
    return RelativeStatus::UNSET;
  }
  if (TargetType == TargetTypeT::String) {
    if (memcmp(new_bytes, old_bytes, compare_size))
      return RelativeStatus::CHANGED;
    else
      return RelativeStatus::UNCHANGED;
  }

  RelativeStatus return_type;
  dispatchType(TargetType, [&]<typename T>() {
    if constexpr (!std::is_same_v<std::string, T>)
      return_type = compareValues<T>(new_bytes, old_bytes);
  });
  return return_type;
}

std::vector<uint8_t> findBytesAround(const uint32_t offset,
                                     const std::vector<uint8_t> &data,
                                     const uint32_t size) {
  uint64_t START = offset < 32 ? 0 : offset - 32;
  uint64_t END =
      offset + 32 + size > data.size() ? data.size() : offset + 32 + size;

  std::vector<uint8_t> bytes(BYTES_BEFORE + BYTES_AFTER + size);
  memcpy(bytes.data(), &data[START], END - START);
  return bytes;
}

template <typename T>
std::vector<uint64_t> searchValue(const std::vector<uint8_t> &Data, T Target,
                                  float epsilon) {

  static_assert(std::is_arithmetic_v<T>,
                "SearchValue only works with numeric types");

  std::vector<uint64_t> FoundOffsets;

  T data_value;

  for (uint32_t i = 0; i + sizeof(T) <= Data.size(); i += sizeof(T)) {
    memcpy(&data_value, Data.data() + i, sizeof(T));
    if constexpr (std::is_floating_point_v<T>) {
      if (std::abs(data_value - Target) <= epsilon)
        FoundOffsets.push_back(i);
    } else {
      if (data_value == Target)
        FoundOffsets.push_back(i);
    }
  }
  return FoundOffsets;
}

std::vector<uint64_t>
searchRawValue(const std::vector<uint8_t> &Data,
               const std::vector<uint8_t> &TargetData,
               const std::vector<bool>
                   &validBytes) { // validbytes for byte scanning later.

  std::vector<uint64_t> FoundOffsets;
  uint64_t TargetSize = TargetData.size();

  for (uint32_t i = 0; i + TargetSize <= Data.size(); ++i) {
    if (memcmp(&Data[i], TargetData.data(), TargetSize) == 0) {
      FoundOffsets.push_back(i);
    }
  }
  return FoundOffsets;
}

std::string DataToStr(const std::vector<uint8_t> &Bytes,
                      const TargetTypeT TargetType) {

  if (Bytes.empty())
    return "";
  switch (TargetType) {
  case TargetTypeT::uInt8:
    return std::to_string(readAs<uint8_t>(Bytes));
  case TargetTypeT::uInt16:
    return std::to_string(readAs<uint16_t>(Bytes));
  case TargetTypeT::uInt32:
    return std::to_string(readAs<uint32_t>(Bytes));
  case TargetTypeT::uInt64:
    return std::to_string(readAs<uint64_t>(Bytes));
  case TargetTypeT::Int8:
    return std::to_string(readAs<int8_t>(Bytes));
  case TargetTypeT::Int16:
    return std::to_string(readAs<int16_t>(Bytes));
  case TargetTypeT::Int32:
    return std::to_string(readAs<int32_t>(Bytes));
  case TargetTypeT::Int64:
    return std::to_string(readAs<int64_t>(Bytes));
  case TargetTypeT::Float:
    return std::to_string(readAs<float>(Bytes));
  case TargetTypeT::Double:
    return std::to_string(readAs<double>(Bytes));
  case TargetTypeT::String:
    return std::string(reinterpret_cast<const char *>(Bytes.data()),
                       Bytes.size());
  default:
    Log::Error("Why is TargetType undefined in HitValToStr?? (TargetType: " +
               std::to_string(static_cast<int>(TargetType)));
    return {};
  }
}

std::string TargetTypeToStr(const TargetTypeT TargetType) {
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

std::string RelativeStatusToStr(const RelativeStatus Status) {
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
    return "Unset...";
  }
  return "";
}

template <typename T> T readAs(const std::vector<uint8_t> &buffer) {
  T This{};

  static_assert(std::is_arithmetic_v<T>,
                "CompareValues only works with numeric types");

  if (buffer.size() >= sizeof(T))
    memcpy(&This, buffer.data(), sizeof(This));

  return This;
}

// this shouldn't really be called by itself. unsafe. Call tagChange instead.
// (not that tagChange is super safe by the way.)
template <typename T>
RelativeStatus compareValues(const char *old_bytes, const char *new_bytes,
                             float epsilon) {

  static_assert(std::is_arithmetic_v<T>,
                "CompareValues only works with numeric types");

  T newval{};
  T oldval{};

  memcpy(&newval, old_bytes, sizeof(newval));
  memcpy(&oldval, new_bytes, sizeof(oldval));

  if constexpr (std::is_floating_point_v<T>) {
    if (std::isnan(newval) || std::isnan(oldval))
      return RelativeStatus::CHANGED;

    float diff = newval - oldval;

    if (std::abs(newval - oldval) <= epsilon)
      return RelativeStatus::UNCHANGED;

    if (diff > 0)
      return RelativeStatus::INCREASED;

    return RelativeStatus::DECREASED;
  }
  if (newval == oldval)
    return RelativeStatus::UNCHANGED;
  if (newval > oldval)
    return RelativeStatus::INCREASED;

  // NaN is not implemented.
  return RelativeStatus::DECREASED;
}

//

template std::vector<uint64_t>
searchValue<uint8_t>(const std::vector<uint8_t> &, uint8_t, float);
template std::vector<uint64_t>
searchValue<uint16_t>(const std::vector<uint8_t> &, uint16_t, float);
template std::vector<uint64_t>
searchValue<uint32_t>(const std::vector<uint8_t> &, uint32_t, float);
template std::vector<uint64_t>
searchValue<uint64_t>(const std::vector<uint8_t> &, uint64_t, float);
template std::vector<uint64_t> searchValue<int8_t>(const std::vector<uint8_t> &,
                                                   int8_t, float);
template std::vector<uint64_t>
searchValue<int16_t>(const std::vector<uint8_t> &, int16_t, float);
template std::vector<uint64_t>
searchValue<int32_t>(const std::vector<uint8_t> &, int32_t, float);
template std::vector<uint64_t>
searchValue<int64_t>(const std::vector<uint8_t> &, int64_t, float);
template std::vector<uint64_t> searchValue<float>(const std::vector<uint8_t> &,
                                                  float, float);
template std::vector<uint64_t> searchValue<double>(const std::vector<uint8_t> &,
                                                   double, float);

//

template RelativeStatus compareValues<uint8_t>(const char *, const char *,
                                               float);
template RelativeStatus compareValues<uint16_t>(const char *, const char *,
                                                float);
template RelativeStatus compareValues<uint32_t>(const char *, const char *,
                                                float);
template RelativeStatus compareValues<uint64_t>(const char *, const char *,
                                                float);
template RelativeStatus compareValues<int8_t>(const char *, const char *,
                                              float);
template RelativeStatus compareValues<int16_t>(const char *, const char *,
                                               float);
template RelativeStatus compareValues<int32_t>(const char *, const char *,
                                               float);
template RelativeStatus compareValues<int64_t>(const char *, const char *,
                                               float);
template RelativeStatus compareValues<float>(const char *, const char *, float);
template RelativeStatus compareValues<double>(const char *, const char *,
                                              float);
