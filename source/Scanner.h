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
  static std::vector<uint32_t>
  SearchValue(const std::vector<uint8_t> &Data,
              const std::vector<uint8_t> &TargetData);
  static std::vector<uint8_t> FindBytesAround(uint32_t offset,
                                              const std::vector<uint8_t> &data,
                                              uint32_t Size);

  template <typename T, typename entry_type>
  static RelativeStatus CompareValues(const entry_type &entry);

  template <typename T> void TagEntryChange(T &entry, TargetTypeT TargetType);

  template <typename T> void RescanEntryData(T &entry);

  template <typename T>
  void RescanEntry(T &entry, const TargetTypeT &TargetType);

  template <typename T>
  void WriteEntryAdr(T entry, const std::vector<uint8_t> &value);

  std::vector<FavouriteInfoT> Favourites;
  std::mutex FavouriteMutex;

  std::atomic<bool> FreezeRunning = false;

  std::thread FreezeThread;
  std::thread ScannerThread;

  void WriteFreezeValues();

  std::chrono::steady_clock::time_point SinceHitRefresh;

public:
  Scanner() = default;
  Scanner(const Scanner &) = delete;
  Scanner &operator=(const Scanner &) = delete;
  Scanner(Scanner &&) = delete;
  Scanner &operator=(Scanner &&) = delete;

  ~Scanner() { FullClear(); }

  void FullClear();

  void RestartClear();

  std::atomic<int64_t> HitRefreshInterval = -1;

  std::vector<HitInfoT> Hits;

  std::atomic<bool> IsScanning{false};
  std::atomic<uint64_t> TotalLoad{0};
  std::atomic<uint64_t> CurrentProgress{0};

  void Init(int pid);

  void StartScan(const TargetInfoT &TargetInfo);

  void RescanHit(uint64_t index, const TargetTypeT &TargetType);
  void RescanAllHits(const TargetTypeT &TargetType);
  void WriteHit(uint64_t index, const std::vector<uint8_t> &value);
  void FilterHit(RelativeStatus Keep1);
  void FilterHit(const std::vector<uint8_t> &keepValue);

  void AddToFavourite(uint64_t index, TargetTypeT TargetType);
  void RemoveFromFavourite(uint64_t index);
  std::vector<FavouriteInfoT> GetFavourites();
  void RescanFavourite(uint64_t index);
  void RescanAllFavourites();
  void WriteFavourite(uint64_t index, const std::vector<uint8_t> &value);
  void SetFreezeFavourite(uint64_t index, bool SetTo);
  void SetDescriptionFavourite(uint64_t index, const std::string &desc);
  void SetRefreshDurationFavourite(uint64_t index, float Duration);

  void StartFreezeThread();
  void EndFreezeThread();

  void RunOnScannerThread(const std::function<void(Scanner &)> &task);

  void AutoRefreshHits(std::chrono::steady_clock::time_point LoopTime,
                       const TargetTypeT &TargetType);

  void AutoRefreshFavourites(std::chrono::steady_clock::time_point LoopTime);
};
