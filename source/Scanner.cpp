#include "Scanner.h"

#include "LogW.h"
#include "platform/ActOS.h"
#include "types.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

void Scanner::Start(int pid) { proc_ = ActOS::Attach(pid); }

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
                         const uint32_t TargetSize) {
  uint64_t START = offset < 32 ? 0 : offset - 32;
  uint64_t END = offset + 32 + TargetSize > data.size()
                     ? data.size()
                     : offset + 32 + TargetSize;

  std::vector<uint8_t> bytes(END - START);

  memcpy(bytes.data(), &data[START], END - START);
  return bytes;
}

// we should also add something that also rescans maps and deals with hits from
// there...at some point.

// oh and a way to compare old and new bytes around? I'd also need to add that
// to hitinfo struct.

void Scanner::RescanHit(uint64_t index) {

  Hits[index].previous_value = Hits[index].value;

  Hits[index].bytes_around =
      proc_->read(Hits[index].location - 32,
                  Hits[index].value.size() + 64); // we really n
  memcpy(Hits[index].value.data(), Hits[index].bytes_around.data() + 32,
         Hits[index].value.size());
  return;
}

// unfinished. for next time.
void Scanner::TagHit(const uint64_t index, const TargetInfoT &TargetInfo) {}