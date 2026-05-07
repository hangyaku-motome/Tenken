#include "Scanner.h"

#include "LogW.h"
#include "platform/ActOS.h"
#include "types.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

void Scanner::Init(int pid) { proc_ = ActOS::Attach(pid); }

void Scanner::StartScan(const TargetInfoT &TargetInfo) { // FirstScan ?
  Hits.clear();
  std::vector<MapInfoT> Maps = proc_->getRegions();
  if (Maps.empty())
    return;

  Log::Info(std::to_string(Maps.size()) + " map regions found.");

  for (const auto &Map : Maps) {

    std::vector<uint8_t> Data = proc_->read(Map.start, Map.end - Map.start);

    for (auto RelativeOffset : SearchValue(Data, TargetInfo.value)) {
      Hits.push_back(
          {TargetInfo.value,
           {},
           Map.start + RelativeOffset,
           FindBytesAround(RelativeOffset, Data, TargetInfo.value.size())});
    }
  }

  Log::Info(std::to_string(Hits.size()) + " hits found.");
}

std::vector<uint32_t>
Scanner::SearchValue(const std::vector<uint8_t> &Data,
                     const std::vector<uint8_t> &TargetData) {

  std::vector<uint32_t> FoundOffsets;
  uint32_t TargetSize = TargetData.size();

  for (uint32_t i = 0; i + TargetSize <= Data.size(); ++i) {
    if (!memcmp(&Data[i], TargetData.data(), TargetSize)) {
      FoundOffsets.push_back(i);
    }
  }
  return FoundOffsets;
}

const std::vector<uint8_t>
Scanner::FindBytesAround(const uint32_t offset,
                         const std::vector<uint8_t> &data,
                         const uint32_t Size) {
  uint64_t START = offset < 32 ? 0 : offset - 32;
  uint64_t END =
      offset + 32 + Size > data.size() ? data.size() : offset + 32 + Size;

  std::vector<uint8_t> bytes(END - START);

  memcpy(bytes.data(), &data[START], END - START);
  return bytes;
}

// we should also add something that also rescans maps and deals with hits from
// there...at some point.

// oh and a way to compare old and new bytes around? I'd also need to add that
// to hitinfo struct.

void Scanner::RescanHitData(uint64_t index) {

  Hits[index].previous_value = Hits[index].value;

  Hits[index].bytes_around =
      proc_->read(Hits[index].location - 32,
                  Hits[index].value.size() + 64); // we really n
  memcpy(Hits[index].value.data(), Hits[index].bytes_around.data() + 32,
         Hits[index].value.size());
  return;
}

template <typename T>
RelativeStatus Scanner::CompareValues(const HitInfoT &Hit) {
  T newval{};
  T oldval{};

  static_assert(std::is_arithmetic_v<T>,
                "CompareValues only works with numeric types");

  memcpy(&newval, Hit.value.data(), Hit.value.size());
  memcpy(&oldval, Hit.previous_value.data(), Hit.value.size());

  if (newval == oldval)
    return RelativeStatus::UNCHANGED;
  if (newval > oldval)
    return RelativeStatus::INCREASED;
  if (newval < oldval)
    return RelativeStatus::DECREASED;

  return RelativeStatus::CHANGED;
}

void Scanner::TagHitChange(const uint64_t index,
                           const TargetInfoT &TargetInfo) {
  if (TargetInfo.TargetType == TargetTypeT::Invalid) {
    Log::Error("Target type is invalid in TagHit, index is: " +
               std::to_string(index));
    return;
  }
  if (TargetInfo.TargetType == TargetTypeT::String) {
    if (!memcmp(Hits[index].value.data(), Hits[index].previous_value.data(),
                Hits[index].value.size()))
      Hits[index].Status = RelativeStatus::CHANGED;
    else
      Hits[index].Status = RelativeStatus::UNCHANGED;

    return;
  }

  RelativeStatus Relativestatus = RelativeStatus::UNSET;

  switch (TargetInfo.TargetType) {
  case TargetTypeT::Int8:
    Relativestatus = CompareValues<int8_t>(Hits[index]);
    break;
  case TargetTypeT::Int16:
    Relativestatus = CompareValues<int16_t>(Hits[index]);
    break;
  case TargetTypeT::Int32:
    Relativestatus = CompareValues<int32_t>(Hits[index]);
    break;
  case TargetTypeT::Int64:
    Relativestatus = CompareValues<int64_t>(Hits[index]);
    break;
  case TargetTypeT::uInt8:
    Relativestatus = CompareValues<uint8_t>(Hits[index]);
    break;
  case TargetTypeT::uInt16:
    Relativestatus = CompareValues<uint16_t>(Hits[index]);
    break;
  case TargetTypeT::uInt32:
    Relativestatus = CompareValues<uint32_t>(Hits[index]);
    break;
  case TargetTypeT::uInt64:
    Relativestatus = CompareValues<uint64_t>(Hits[index]);
    break;
  case TargetTypeT::Float:
    Relativestatus = CompareValues<float>(Hits[index]);
    break;
  case TargetTypeT::Double:
    Relativestatus = CompareValues<double>(Hits[index]);
    break;
  default:
    Log::Error("In TagHitChange why is TargetType invalid?");
    break;
  }

  Hits[index].Status = Relativestatus;
}

void Scanner::FilterHit(const RelativeStatus Keep1) {
  RelativeStatus Keep2 = Keep1;
  RelativeStatus Keep3 = Keep1;
  if (Keep1 == RelativeStatus::CHANGED) {
    Keep2 = RelativeStatus::INCREASED;
    Keep3 = RelativeStatus::DECREASED;
  }

  Hits.erase(std::remove_if(Hits.begin(), Hits.end(),
                            [Keep1, Keep2, Keep3](const HitInfoT &hit) {
                              return hit.Status != Keep1 &&
                                     hit.Status != Keep2 && hit.Status != Keep3;
                            }),
             Hits.end());
}

void Scanner::FilterHit(const std::vector<uint8_t> &keepValue) {

  Hits.erase(std::remove_if(Hits.begin(), Hits.end(),
                            [keepValue](const HitInfoT &hit) {
                              return memcmp(hit.value.data(), keepValue.data(),
                                            keepValue.size()) != 0;
                            }),
             Hits.end());
}