#include "utils.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "misc/cpp/imgui_stdlib.h"
#include "types.h"

template <typename T> RelativeStatus tagChange(T new_value, T old_value) {
  if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<uint8_t>>) {
    if (old_value != new_value)
      return RelativeStatus::CHANGED;
    else
      return RelativeStatus::UNCHANGED;
  } else if constexpr (std::is_floating_point_v<T>) {
    if (std::isnan(old_value) || std::isnan(new_value)) return RelativeStatus::CHANGED;

    float diff = old_value - new_value;
    if (std::abs(diff) <= EPSILON) return RelativeStatus::UNCHANGED;

    if (diff > 0) return RelativeStatus::INCREASED;

    return RelativeStatus::DECREASED;
  } else if constexpr (std::is_arithmetic_v<T>) {
    if (old_value == new_value) return RelativeStatus::UNCHANGED;
    if (new_value > old_value) return RelativeStatus::INCREASED;

    return RelativeStatus::DECREASED;
  }

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

std::vector<uint8_t> findBytesAround(const uint32_t offset, const std::vector<uint8_t>& data, const uint32_t size) {
  uint64_t START = offset < 32 ? 0 : offset - 32;
  uint64_t END = offset + 32 + size > data.size() ? data.size() : offset + 32 + size;

  std::vector<uint8_t> bytes(BYTES_BEFORE + BYTES_AFTER + size);
  memcpy(bytes.data(), &data[START], END - START);
  return bytes;
}

// I could simplfy this my merging string path and primitive, as the only difference is size.
template <typename T>
std::vector<uint64_t> searchValue(const std::vector<uint8_t>& Data, const T& Target, const std::vector<bool>& mask) {
  std::vector<uint64_t> FoundOffsets;

  T data_value;

  if constexpr (std::is_same_v<std::string, T>) {
    for (uint32_t i = 0; i + Target.size() <= Data.size(); ++i)
      if (memcmp(&Data[i], Target.data(), Target.size()) == 0) FoundOffsets.push_back(i);

  } else if constexpr (std::is_same_v<std::vector<uint8_t>, T>) {
    for (uint32_t i = 0; i + Target.size() <= Data.size(); ++i) {
      bool push = true;
      for (uint32_t k = 0; k < Target.size(); ++k)
        if (memcmp(Data.data() + i + k, Target.data() + k, 1) != 0 && mask[k]) {
          push = false;
          break;
        }

      if (push) FoundOffsets.push_back(i);
    }
  } else {
    for (uint32_t i = 0; i + sizeof(T) <= Data.size(); i += sizeof(T)) {
      memcpy(&data_value, Data.data() + i, sizeof(T));
      if constexpr (std::is_floating_point_v<T>) {
        if (std::abs(data_value - Target) <= EPSILON) FoundOffsets.push_back(i);
      } else {
        if (data_value == Target) FoundOffsets.push_back(i);
      }
    }
  }
  return FoundOffsets;
}

template std::vector<uint64_t>
searchValue<uint8_t>(const std::vector<uint8_t>&, const uint8_t&, const std::vector<bool>&);
template std::vector<uint64_t>
searchValue<uint16_t>(const std::vector<uint8_t>&, const uint16_t&, const std::vector<bool>&);
template std::vector<uint64_t>
searchValue<uint32_t>(const std::vector<uint8_t>&, const uint32_t&, const std::vector<bool>&);
template std::vector<uint64_t>
searchValue<uint64_t>(const std::vector<uint8_t>&, const uint64_t&, const std::vector<bool>&);
template std::vector<uint64_t>
searchValue<int8_t>(const std::vector<uint8_t>&, const int8_t&, const std::vector<bool>&);
template std::vector<uint64_t>
searchValue<int16_t>(const std::vector<uint8_t>&, const int16_t&, const std::vector<bool>&);
template std::vector<uint64_t>
searchValue<int32_t>(const std::vector<uint8_t>&, const int32_t&, const std::vector<bool>&);
template std::vector<uint64_t>
searchValue<int64_t>(const std::vector<uint8_t>&, const int64_t&, const std::vector<bool>&);
template std::vector<uint64_t> searchValue<float>(const std::vector<uint8_t>&, const float&, const std::vector<bool>&);
template std::vector<uint64_t>
searchValue<double>(const std::vector<uint8_t>&, const double&, const std::vector<bool>&);
template std::vector<uint64_t>
searchValue<std::string>(const std::vector<uint8_t>&, const std::string&, const std::vector<bool>&);
template std::vector<uint64_t>
searchValue<std::vector<uint8_t>>(const std::vector<uint8_t>&, const std::vector<uint8_t>&, const std::vector<bool>&);

//

// I might wanna merge searchRawValue and searchValue but not right now.

std::vector<uint64_t> searchRawValue(const std::vector<uint8_t>& Data,
                                     const std::vector<uint8_t>& TargetData,
                                     const std::vector<bool>& mask) {  // validbytes for byte scanning later.

  std::vector<uint64_t> FoundOffsets;
  uint64_t TargetSize = TargetData.size();

  for (uint32_t i = 0; i + TargetSize <= Data.size(); ++i) {
    if (memcmp(&Data[i], TargetData.data(), TargetSize) == 0) {
      FoundOffsets.push_back(i);
    }
  }
  return FoundOffsets;
}

// a function that takes no mask just returns a normal string for vector.
// a wrapper function that swtiches the bytes to ?? as neeeded:

std::string dataToMaskedStr(const std::vector<uint8_t>& bytes, const std::vector<bool>& mask) {
  std::string returnstring;
  for (uint64_t i = 0; i < bytes.size(); ++i) {
    if (i != 0) returnstring += " ";
    if (mask[i] == false) {
      returnstring += "??";
      continue;
    }
    returnstring += hexToStr(bytes[i]);
  }
  return returnstring;
}

template <typename T> std::string dataToStr(const std::vector<uint8_t>& bytes) {
  if (bytes.empty()) return "";

  if constexpr (std::is_same_v<T, std::string>) {
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
  } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
    std::string returnstring;
    for (uint64_t i = 0; i < bytes.size(); ++i) {
      if (i != 0) returnstring += " ";
      returnstring += hexToStr(bytes[i]);
    }
    return returnstring;
  } else {
    T value;
    memcpy(&value, bytes.data(), sizeof(T));
    std::string returnstring = std::to_string(value);

    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
      char buf[64];
      snprintf(buf, sizeof(buf), "%.6g", value);
      return std::string(buf);
    }

    return returnstring;
  }
}

template std::string dataToStr<uint8_t>(const std::vector<uint8_t>&);
template std::string dataToStr<uint16_t>(const std::vector<uint8_t>&);
template std::string dataToStr<uint32_t>(const std::vector<uint8_t>&);
template std::string dataToStr<uint64_t>(const std::vector<uint8_t>&);
template std::string dataToStr<int8_t>(const std::vector<uint8_t>&);
template std::string dataToStr<int16_t>(const std::vector<uint8_t>&);
template std::string dataToStr<int32_t>(const std::vector<uint8_t>&);
template std::string dataToStr<int64_t>(const std::vector<uint8_t>&);
template std::string dataToStr<float>(const std::vector<uint8_t>&);
template std::string dataToStr<double>(const std::vector<uint8_t>&);
template std::string dataToStr<std::string>(const std::vector<uint8_t>&);
template std::string dataToStr<std::vector<uint8_t>>(const std::vector<uint8_t>&);

//

std::string targetTypeToStr(const TargetTypeT targetType) {
  switch (targetType) {
    case TargetTypeT::uInt8:
      return "uInt8";
    case TargetTypeT::uInt16:
      return "uInt16";
    case TargetTypeT::uInt32:
      return "uInt32";
    case TargetTypeT::uInt64:
      return "uInt64";
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
    case TargetTypeT::Invalid:
      return "Invalid";
    case TargetTypeT::AOB:
      return "AOB";
  }
}

TargetTypeT strToTargetType(const std::string &string) {
  if(string == "uInt8") return TargetTypeT::uInt8;
  if(string == "uInt16") return TargetTypeT::uInt16;
  if(string == "uInt32") return TargetTypeT::uInt32;
  if(string == "uint64") return TargetTypeT::uInt64;
  if(string == "Int8") return TargetTypeT::Int8;
  if(string == "Int16") return TargetTypeT::Int16;
  if(string == "Int32") return TargetTypeT::Int32;
  if(string == "Int64") return TargetTypeT::Int64;

  return TargetTypeT::Invalid;
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
template <typename T> T dataToType(const std::vector<uint8_t>& data) {
  if constexpr (std::is_same_v<T, std::string>) {
    return std::string(reinterpret_cast<const char*>(data.data()), data.size());
  } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
    return data;
  } else {
    T returnVal;
    memcpy(&returnVal, data.data(), sizeof(T));
    return returnVal;
  }
}

template uint8_t dataToType<uint8_t>(const std::vector<uint8_t>&);
template uint16_t dataToType<uint16_t>(const std::vector<uint8_t>&);
template uint32_t dataToType<uint32_t>(const std::vector<uint8_t>&);
template uint64_t dataToType<uint64_t>(const std::vector<uint8_t>&);
template int8_t dataToType<int8_t>(const std::vector<uint8_t>&);
template int16_t dataToType<int16_t>(const std::vector<uint8_t>&);
template int32_t dataToType<int32_t>(const std::vector<uint8_t>&);
template int64_t dataToType<int64_t>(const std::vector<uint8_t>&);
template float dataToType<float>(const std::vector<uint8_t>&);
template double dataToType<double>(const std::vector<uint8_t>&);
template std::vector<uint8_t> dataToType<std::vector<uint8_t>>(const std::vector<uint8_t>&);
template std::string dataToType<std::string>(const std::vector<uint8_t>&);

//

bool strToAOBInfo(std::vector<uint8_t>& bytes, std::vector<bool>& mask) {
  std::string tempbuf = dataToMaskedStr(bytes, mask);
  if (!ImGui::InputText("##value", &tempbuf)) {
    if (tempbuf.empty()) bytes.clear();
    return false;
  }

  std::istringstream stream(tempbuf);
  std::string token;

  std::vector<uint8_t> new_bytes;
  std::vector<bool> new_mask;

  while (stream >> token) {
    if (token == "??") {
      new_mask.push_back(false);
      new_bytes.push_back('0');
      continue;
    }

    try {
      new_bytes.push_back(static_cast<uint8_t>(std::stoi(token, nullptr, 16)));
      new_mask.push_back(true);
    } catch (...) {
      return false;
    }
  }

  if (new_mask.empty() || new_bytes.empty()) return false;

  bytes = new_bytes;
  mask = new_mask;

  return true;
}

std::string hexToStr(const uint8_t byte) { return std::string({hex[(byte >> 4)], hex[(byte & 0xF)]}); }
