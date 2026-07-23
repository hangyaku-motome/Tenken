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
      return RelativeStatus::changed;
    else
      return RelativeStatus::unchanged;
  } else if constexpr (std::is_floating_point_v<T>) {
    if (std::isnan(old_value) || std::isnan(new_value)) return RelativeStatus::changed;

    float diff = static_cast<float>(old_value - new_value);
    if (std::abs(diff) <= epsilon) return RelativeStatus::unchanged;

    if (diff > 0) return RelativeStatus::increased;

    return RelativeStatus::decreased;
  } else if constexpr (std::is_arithmetic_v<T>) {
    if (old_value == new_value) return RelativeStatus::unchanged;
    if (new_value > old_value) return RelativeStatus::increased;

    return RelativeStatus::decreased;
  }

  return RelativeStatus::unset;
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

std::vector<uint8_t> findBytesAround(const uint64_t offset, const std::vector<uint8_t>& data, const uint32_t size) {
  uint64_t start = offset < bytes_before ? 0 : offset - bytes_before;
  uint64_t end = offset + bytes_after + size > data.size() ? data.size() : offset + bytes_after + size;

  std::vector<uint8_t> bytes(bytes_before + bytes_after + size);
  memcpy(bytes.data(), &data[start], end - start);
  return bytes;
}

// I could simplfy this my merging string path and primitive, as the only difference is size.
template <typename T>
std::vector<uint64_t> searchValue(const std::vector<uint8_t>& data, const T& target, const std::vector<bool>& mask) {
  std::vector<uint64_t> found_offsets;

  T data_value;

  if constexpr (std::is_same_v<std::string, T>) {
    for (uint32_t i = 0; i + target.size() <= data.size(); ++i)
      if (memcmp(&data[i], target.data(), target.size()) == 0) found_offsets.push_back(i);

  } else if constexpr (std::is_same_v<std::vector<uint8_t>, T>) {
    for (uint32_t i = 0; i + target.size() <= data.size(); ++i) {
      bool push = true;
      for (uint32_t k = 0; k < target.size(); ++k)
        if (memcmp(data.data() + i + k, target.data() + k, 1) != 0 && mask[k]) {
          push = false;
          break;
        }

      if (push) found_offsets.push_back(i);
    }
  } else {
    for (uint32_t i = 0; i + sizeof(T) <= data.size(); i += sizeof(T)) {
      memcpy(&data_value, data.data() + i, sizeof(T));
      if constexpr (std::is_floating_point_v<T>) {
        if (std::abs(data_value - target) <= epsilon) found_offsets.push_back(i);
      } else {
        if (data_value == target) found_offsets.push_back(i);
      }
    }
  }
  return found_offsets;
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

std::string dataToMaskedStr(const std::vector<uint8_t>& bytes, const std::vector<bool>& mask) {
  std::string return_string;
  for (uint64_t i = 0; i < bytes.size(); ++i) {
    if (i != 0) return_string += " ";
    if (mask[i] == false) {
      return_string += "??";
      continue;
    }
    return_string += hexToStr(bytes[i]);
  }
  return return_string;
}

template <typename T> std::string dataToStr(const std::vector<uint8_t>& bytes) {
  if (bytes.empty()) return "";

  if constexpr (std::is_same_v<T, std::string>) {
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
  } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
    std::string return_string;
    for (uint64_t i = 0; i < bytes.size(); ++i) {
      if (i != 0) return_string += " ";
      return_string += hexToStr(bytes[i]);
    }
    return return_string;
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

std::string targetTypeToStr(const TargetType targetType) {
  switch (targetType) {
    case TargetType::uInt8:
      return "uInt8";
    case TargetType::uInt16:
      return "uInt16";
    case TargetType::uInt32:
      return "uInt32";
    case TargetType::uInt64:
      return "uInt64";
    case TargetType::int8:
      return "int8";
    case TargetType::int16:
      return "int16";
    case TargetType::int32:
      return "int32";
    case TargetType::int64:
      return "int64";
    case TargetType::f32:
      return "float";
    case TargetType::f64:
      return "double";
    case TargetType::string:
      return "string";
    case TargetType::invalid:
      return "invalid";
    case TargetType::aob:
      return "aob";
    default:
    return "";
  }
}

TargetType strToTargetType(const std::string& string) {
  if (string == "uInt8") return TargetType::uInt8;
  if (string == "uInt16") return TargetType::uInt16;
  if (string == "uInt32") return TargetType::uInt32;
  if (string == "uInt64") return TargetType::uInt64;
  if (string == "int8") return TargetType::int8;
  if (string == "int16") return TargetType::int16;
  if (string == "int32") return TargetType::int32;
  if (string == "int64") return TargetType::int64;

  return TargetType::invalid;
}

std::string relativeStatusToStr(const RelativeStatus status) {
  switch (status) {
    case RelativeStatus::increased:
      return "Increased";
    case RelativeStatus::decreased:
      return "Decreased";
    case RelativeStatus::unchanged:
      return "Unchanged";
    case RelativeStatus::changed:
      return "Changed";
    case RelativeStatus::unset:
      return "Unset";
    default:
    return "";
  }
}

// end of tostr stuff.

// takes a typename T. takes data. casts it to that. returns that.
template <typename T> T dataToType(const std::vector<uint8_t>& data) {
  if constexpr (std::is_same_v<T, std::string>) {
    return std::string(reinterpret_cast<const char*>(data.data()), data.size());
  } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
    return data;
  } else {
    T return_val;
    memcpy(&return_val, data.data(), sizeof(T));
    return return_val;
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
  std::string tmp_str = dataToMaskedStr(bytes, mask);
  if (!ImGui::InputText("##value", &tmp_str)) {
    if (tmp_str.empty()) bytes.clear();
    return false;
  }

  std::istringstream stream(tmp_str);
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

std::string mapTypeToStr(const MapType type) {
  switch (type) {
    case MapType::mainExecData:
      return "Main Exec Data";
    case MapType::anon:
      return "Anon";
    case MapType::heap:
      return "Heap";
    case MapType::mainExecCode:
      return "Main Exec Code";
    case MapType::mainExecConst:
      return "Main Exec Const";
    case MapType::sharedLibCode:
      return "Shared Lib Code";
    case MapType::sharedLibData:
      return "Shared Lib Data";
    case MapType::sharedLibConst:
      return "Shared Lib Const";
    case MapType::kernelPages:
      return "Kernel Pages";
    case MapType::stack:
      return "Stack";
    case MapType::unreadable:
      return "Unreadable";
    case MapType::unset:
      return "Unset";
    default:
    return "";
  }
}

int64_t signedDiff(uint64_t a, uint64_t b) {
  return (a >= b) ? static_cast<int64_t>(a - b) : -static_cast<int64_t>(b - a);
}
