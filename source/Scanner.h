#pragma once

#include "platform/ActOS.h"
#include "types.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <memory>
#include <vector>

class Scanner {

private:
  std::unique_ptr<ActOS::IProcess> proc_ = nullptr;
  std::vector<uint32_t> SearchValue(const std::vector<uint8_t> &Data,
                                    const std::vector<uint8_t> &TargetData);

  const std::vector<uint8_t> FindBytesAround(const uint32_t offset,
                                             const std::vector<uint8_t> &data,
                                             const uint32_t Size);
  template <typename T> RelativeStatus CompareValues(const HitInfoT &Hit);

  template <typename T>
  void RescanEntryData(std::vector<T> &entries, const uint64_t index);
  template <typename T>
  void TagEntryChange(std::vector<T> &entries, const uint64_t index,
                      const TargetInfoT &TargetInfo);

public:
  Scanner() {};

  std::vector<HitInfoT> Hits;
  std::vector<FavouriteInfoT> Favourites;

  void Init(int pid);

  void StartScan(const TargetInfoT &TargetInfo);

  void RescanHit(const uint64_t index, const TargetInfoT &TargetInfo);
  void RescanFavourite(const uint64_t index, const TargetInfoT &TargetInfo);
  void AddToFavourite(const uint64_t index);
  void WriteAdr(const uint64_t index,
                const std::vector<uint8_t> &value); // unfinished right now.
  void FilterHit(const RelativeStatus Keep1);
  void FilterHit(const std::vector<uint8_t> &keepValue);
};
