#pragma once

#include "platform/ActOS.h"
#include "types.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class Scanner {

private:
  std::unique_ptr<ActOS::IProcess> proc_ = nullptr;
  std::vector<uint32_t> SearchValue(const std::vector<uint8_t> &Data,
                                    const std::vector<uint8_t> &TargetData);
  const std::vector<uint8_t> FindBytesAround(const uint32_t offset,
                                             const std::vector<uint8_t> &data,
                                             const uint32_t Size);

  template <typename T, typename entry_type>
  RelativeStatus CompareValues(const entry_type &entry);

  template <typename T>
  void TagEntryChange(T &entry, const TargetTypeT TargetType);

  template <typename T> void RescanEntryData(T &entry);

  template <typename T>
  void RescanEntry(T &entry, const TargetTypeT &TargetType);

  template <typename T>
  void WriteEntryAdr(const T entry, const std::vector<uint8_t> &value);

  // favourite should always follow mutex!
  std::vector<FavouriteInfoT> Favourites;
  std::mutex FavouriteMutex;

  std::atomic<bool> FreezeRunning = false;

  std::thread FreezeThread;

  void WriteFreezeValues();

public:
  Scanner() {};

  void Clear() {
    Hits.clear();
    EndFreezeThread();
    std::scoped_lock<std::mutex> lock(
        FavouriteMutex); // feels useless but oh well.
    Favourites.clear();
    IsScanning = false;
  }

  std::vector<HitInfoT> Hits;

  std::atomic<bool> IsScanning{false};
  std::atomic<int64_t> TotalLoad{0};
  std::atomic<int64_t> CurrentProgress{0};

  void Init(int pid);

  void StartScan(const TargetInfoT &TargetInfo);

  void AddToFavourite(const uint64_t index); // mutex

  // I made wrappers cause it felt weird to otherwise call scanner from main
  // with one of it's own variables.
  void RescanHit(const uint64_t index, const TargetTypeT &TargetType);
  void RescanAllHits(const TargetTypeT &TargetType) {
    IsScanning = true;
    TotalLoad = Hits.size();
    for (CurrentProgress = 0; CurrentProgress < Hits.size(); ++CurrentProgress)
      RescanHit(CurrentProgress, TargetType);
    IsScanning = false;
  }

  void RescanFavourite(const uint64_t index,
                       const TargetTypeT &TargetType); // mutex

  std::vector<FavouriteInfoT> GetFavourites() { // mutex
    std::scoped_lock<std::mutex> lock(FavouriteMutex);
    return Favourites;
  }

  void WriteFavourite(const uint64_t index,
                      const std::vector<uint8_t> &value); // mutex

  void SetFavouriteFreeze(const uint64_t index, const bool SetTo) { // mutex
    std::scoped_lock<std::mutex> lock(FavouriteMutex);

    Favourites[index].Frozen = SetTo;
  }
  void SetDescriptionFavourite(const uint64_t index,
                               const std::string desc) { // mutex
    std::scoped_lock<std::mutex> lock(FavouriteMutex);
    Favourites[index].Description = desc;
  }

  void WriteHit(const uint64_t index, const std::vector<uint8_t> &value);

  void FilterHit(const RelativeStatus Keep1);
  void FilterHit(const std::vector<uint8_t> &keepValue);

  void StartFreezeThread();
  void EndFreezeThread();
};
