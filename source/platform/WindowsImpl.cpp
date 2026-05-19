#include "ActOS.h"
#include "types.h"
#include <cstdint>
#include <handleapi.h>
#include <memoryapi.h>
#include <minwindef.h>
#include <psapi.h>
#include <stdio.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <vector>
#include <windows.h>
#include <winnt.h>

namespace ActOS {
std::vector<ProcessInfoT> GetProcTargets();

namespace {

class WindowsImpl : public IProcess {
  HANDLE handle_ = nullptr;

public:
  WindowsImpl(int32_t pid) {
    handle_ = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE |
                              PROCESS_QUERY_INFORMATION,
                          FALSE, static_cast<uint32_t>(pid));
  }

  ~WindowsImpl() {
    if (handle_)
      CloseHandle(handle_);
  }

  std::vector<MapInfoT> getRegions() override;
  std::vector<uint8_t> read(uint64_t address, uint64_t ReadSize) override;
  bool write(uint64_t address, const std::vector<uint8_t> &value) override;
  char *AllocMMapDisk(uint64_t size) override;
  void UnAllocMMapDisk(uint64_t address, uint64_t size) override;

}; // namespace WindowsImpl IProcess

void WindowsImpl::UnAllocMMapDisk(uint64_t address, uint64_t size) {
  VirtualFree(reinterpret_cast<void *>(address), 0, MEM_RELEASE);
}

std::vector<MapInfoT> WindowsImpl::getRegions() {
  MEMORY_BASIC_INFORMATION regioninfo;
  LPVOID address = 0;

  std::vector<MapInfoT> Maps;
  while (VirtualQueryEx(handle_, address, &regioninfo, sizeof(regioninfo))) {

    if (regioninfo.State == MEM_COMMIT && !(regioninfo.Protect & PAGE_NOACCESS) &&
        !(regioninfo.Protect & PAGE_GUARD)) {
      MapInfoT PushMap;
      PushMap.start = reinterpret_cast<uint64_t>(regioninfo.BaseAddress);
      PushMap.end = regioninfo.RegionSize + PushMap.start;
      wchar_t filename[MAX_PATH];
      if (K32GetMappedFileNameW(handle_, regioninfo.BaseAddress, filename, MAX_PATH)) {
        std::wstring ws(filename);
        PushMap.name = {ws.begin(), ws.end()};
      } else
        PushMap.name = "UNNAMED_REGION";

      Maps.push_back(PushMap);
    }

    address = reinterpret_cast<LPVOID>(reinterpret_cast<uint64_t>(regioninfo.BaseAddress) +
                                       regioninfo.RegionSize);
  }

  return Maps;
}

std::vector<uint8_t> WindowsImpl::read(uint64_t address, uint64_t ReadSize) {
  std::vector<uint8_t> readBytes(ReadSize);

  bool res = ReadProcessMemory(handle_, reinterpret_cast<void *>(address), readBytes.data(),
                               ReadSize, NULL);

  if (res == 0) {
    printf("couldn't read\n");
    return {};
  }
  return readBytes;
}

bool WindowsImpl::write(uint64_t address, const std::vector<uint8_t> &value) {

  uint64_t bytes_written;
  bool res = WriteProcessMemory(handle_, reinterpret_cast<void *>(address), value.data(),
                                value.size(), &bytes_written);

  if (res == 0)
    printf("write failed\n");

  return res && bytes_written == value.size();
}

char *WindowsImpl::AllocMMapDisk(uint64_t size) {

  char *ret =
      static_cast<char *>(VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

  if (ret == NULL) {
    printf("alloc failed.·\n");
  }

  return ret;
}

}; // namespace

std::vector<ProcessInfoT> GetProcTargets() {
  HANDLE hProcessSnap;
  PROCESSENTRY32 pe32;

  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  pe32.dwSize = sizeof(PROCESSENTRY32);

  if (!Process32First(hProcessSnap, &pe32)) {
    printf("failed to get processes\n");
    CloseHandle(hProcessSnap); // clean the snapshot object
    return {};
  }

  std::vector<ProcessInfoT> Proccesses;
  do {
    Proccesses.push_back({static_cast<int32_t>(pe32.th32ProcessID), pe32.szExeFile, ""});
  } while (Process32Next(hProcessSnap, &pe32));

  CloseHandle(hProcessSnap);
  return Proccesses;
}

std::unique_ptr<IProcess> Attach(int pid) { return std::make_unique<WindowsImpl>(pid); }

} // namespace ActOS
