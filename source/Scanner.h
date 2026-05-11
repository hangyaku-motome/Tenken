#pragma once

#include "platform/ActOS.h"
#include "types.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <atomic>
#include <cstdint>
#include <functional>
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

  std::vector<FavouriteInfoT> Favourites;
  std::mutex FavouriteMutex;

  std::atomic<bool> FreezeRunning = false;

  std::thread FreezeThread;
  std::thread ScannerThread;

  void WriteFreezeValues();

  std::chrono::steady_clock::time_point SinceHitRefresh;

public:
  Scanner() {};
  ~Scanner() { FullClear(); }

  void FullClear();

  void RestartClear();

  std::atomic<int32_t> HitRefreshInterval = -1;

  std::vector<HitInfoT> Hits;

  std::atomic<bool> IsScanning{false};
  std::atomic<int64_t> TotalLoad{0};
  std::atomic<int64_t> CurrentProgress{0};

  void Init(int pid);

  void StartScan(const TargetInfoT &TargetInfo);

  void RescanHit(const uint64_t index, const TargetTypeT &TargetType);
  void RescanAllHits(const TargetTypeT &TargetType);
  void WriteHit(const uint64_t index, const std::vector<uint8_t> &value);
  void FilterHit(const RelativeStatus Keep1);
  void FilterHit(const std::vector<uint8_t> &keepValue);

  void AddToFavourite(const uint64_t index, const TargetTypeT TargetType);
  std::vector<FavouriteInfoT> GetFavourites();
  void RescanFavourite(const uint64_t index);
  void WriteFavourite(const uint64_t index, const std::vector<uint8_t> &value);
  void SetFreezeFavourite(const uint64_t index, const bool SetTo);
  void SetDescriptionFavourite(const uint64_t index, const std::string desc);
  void SetRefreshDurationFavourite(const uint64_t index, const float Duration);

  void StartFreezeThread();
  void EndFreezeThread();

  void RunOnScannerThread(std::function<void(Scanner &)> task);

  void AutoRefreshHits(const std::chrono::steady_clock::time_point LoopTime,
                       const TargetTypeT &TargetType);

  void
  AutoRefreshFavourites(const std::chrono::steady_clock::time_point LoopTime);
};
