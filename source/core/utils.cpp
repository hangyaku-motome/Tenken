#include "utils.h"
#include "LogW.h"

RelativeStatus tagEntryChange(const std::vector<uint8_t> &new_value,
                              const std::vector<uint8_t> &previous_value,
                              const TargetTypeT TargetType) {
  if (TargetType == TargetTypeT::Invalid) {
    return RelativeStatus::UNSET;
  }
  if (TargetType == TargetTypeT::String) {
    if (memcmp(new_value.data(), previous_value.data(), new_value.size()))
      return RelativeStatus::CHANGED;
    else
      return RelativeStatus::UNCHANGED;
  }

  RelativeStatus return_type;
  dispatchType(TargetType, [&]<typename T>() {
    return_type =
        compareValues<T>(reinterpret_cast<const char *>(new_value.data()),
                         reinterpret_cast<const char *>(previous_value.data()));
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

std::vector<uint64_t> searchValue(const std::vector<uint8_t> &Data,
                                  const std::vector<uint8_t> &TargetData) {

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
