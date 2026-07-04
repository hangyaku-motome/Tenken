#include <handleapi.h>
#include <memoryapi.h>
#include <minwindef.h>
#include <psapi.h>
#include <stdio.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <windows.h>
#include <winnls.h>
#include <winnt.h>

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "LogW.h"
#include "ProcessOS.h"
#include "types.h"

namespace ProcessOS {
std::vector<ProcessInfoT> GetProcTargets();

namespace {

class WindowsImpl : public IProcess {
  HANDLE handle_ = nullptr;

public:
  WindowsImpl(int32_t pid) {
    handle_ = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION,
                          FALSE,
                          static_cast<uint32_t>(pid));
  }

  ~WindowsImpl() {
    if (handle_) CloseHandle(handle_);
  }

  std::vector<MapInfoT> getRegions() override;
  std::vector<uint8_t> read(uint64_t address, uint64_t ReadSize) override;
  bool write(uint64_t address, const std::vector<uint8_t>& value) override;
  char* allocMMapDisk(uint64_t size) override;
  void unAllocMMapDisk(uint64_t address, uint64_t size) override;
  bool isAttached() override;

};  // namespace WindowsImpl IProcess

bool WindowsImpl::isAttached() { return handle_ != nullptr; }

void WindowsImpl::unAllocMMapDisk(uint64_t address, uint64_t size) {
  VirtualFree(reinterpret_cast<void*>(address), 0, MEM_RELEASE);
}

std::vector<MapInfoT> WindowsImpl::getRegions() {
  std::vector<MapInfoT> maps;

  wchar_t main_exe_path[MAX_PATH];
  DWORD path_len = ARRAYSIZE(main_exe_path);
  if (!QueryFullProcessImageNameW(handle_, 0, main_exe_path, &path_len)) {
    return maps;
  }
  std::wstring main_exe(main_exe_path, path_len);

  std::vector<HMODULE> modules(256);
  DWORD needed_bytes = 0;
  if (!EnumProcessModulesEx(handle_,
                            modules.data(),
                            static_cast<DWORD>(modules.size() * sizeof(HMODULE)),
                            &needed_bytes,
                            LIST_MODULES_ALL)) {
    return {};
  }
  if (needed_bytes > modules.size() * sizeof(HMODULE)) {
    modules.resize(needed_bytes / sizeof(HMODULE));
    if (!EnumProcessModulesEx(handle_,
                              modules.data(),
                              static_cast<DWORD>(modules.size() * sizeof(HMODULE)),
                              &needed_bytes,
                              LIST_MODULES_ALL)) {
      return {};
    }
  }
  modules.resize(needed_bytes / sizeof(HMODULE));

  struct ModuleEntry {
    std::wstring path;
    bool is_main;
  };

  std::unordered_map<uint64_t, ModuleEntry> module_map;

  for (HMODULE module : modules) {
    MODULEINFO module_info;
    if (!GetModuleInformation(handle_, module, &module_info, sizeof(module_info))) continue;

    wchar_t mod_path[MAX_PATH];
    if (!GetModuleFileNameExW(handle_, module, mod_path, ARRAYSIZE(mod_path))) continue;

    module_map[reinterpret_cast<uint64_t>(module_info.lpBaseOfDll)] = {mod_path, main_exe == mod_path};
  }

  LPVOID address = nullptr;
  MEMORY_BASIC_INFORMATION region_info;
  while (VirtualQueryEx(handle_, address, &region_info, sizeof(region_info))) {
    LPVOID next_address =
        reinterpret_cast<LPVOID>(reinterpret_cast<uint64_t>(region_info.BaseAddress) + region_info.RegionSize);

    if (region_info.State != MEM_COMMIT || (region_info.Protect & PAGE_NOACCESS) ||
        (region_info.Protect & PAGE_GUARD)) {
      address = next_address;
      continue;
    }

    MapInfoT region;
    region.start = reinterpret_cast<uint64_t>(region_info.BaseAddress);
    region.end = region.start + region_info.RegionSize;

    const bool exec =
        region_info.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
    const bool write =
        region_info.Protect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);

    switch (region_info.Type) {
      case MEM_IMAGE: {
        // HERE.
        auto it = module_map.find(reinterpret_cast<uint64_t>(region_info.AllocationBase));
        const bool is_main = (it != module_map.end()) && it->second.is_main;
        if (it != module_map.end()) {
          int needed = WideCharToMultiByte(CP_UTF8, 0, it->second.path.c_str(), -1, nullptr, 0, nullptr, nullptr);
          if (needed > 0) {
            region.name.resize(needed - 1);
            WideCharToMultiByte(CP_UTF8, 0, it->second.path.c_str(), -1, region.name.data(), needed, nullptr, nullptr);
          }
        }

        if (exec) {
          region.type = is_main ? MapType::MAIN_EXEC_CODE : MapType::SHARED_LIB_CODE;
        } else if (write) {
          region.type = is_main ? MapType::MAIN_EXEC_DATA : MapType::SHARED_LIB_DATA;
        } else {
          region.type = is_main ? MapType::MAIN_EXEC_CONST : MapType::SHARED_LIB_CONST;
        }
        break;
      }

      case MEM_MAPPED: {
        region.type = MapType::ANON;
        wchar_t fname_wstr[MAX_PATH];
        if (K32GetMappedFileNameW(handle_, region_info.BaseAddress, fname_wstr, MAX_PATH)) {
          int needed = WideCharToMultiByte(CP_UTF8, 0, fname_wstr, -1, nullptr, 0, nullptr, nullptr);
          if (needed > 0) {
            region.name.resize(needed - 1);  // -1 because we don't want the null in std::string
            WideCharToMultiByte(CP_UTF8, 0, fname_wstr, -1, region.name.data(), needed, nullptr, nullptr);
          }
        }
        if (region.name.at(0) == '\0') region.name = "UNNAMED_REGION";
        break;
      }
      case MEM_PRIVATE:
        // TODO: no stack or heap
        region.type = MapType::ANON;
        region.name = "UNNAMED_REGION";
        break;
    }

    maps.push_back(std::move(region));
    address = next_address;
  }

  return maps;
}

std::vector<uint8_t> WindowsImpl::read(uint64_t address, uint64_t ReadSize) {
  std::vector<uint8_t> readBytes(ReadSize);

  bool res = ReadProcessMemory(handle_, reinterpret_cast<void*>(address), readBytes.data(), ReadSize, NULL);

  if (res == 0) {
    return {};
  }
  return readBytes;
}

bool WindowsImpl::write(uint64_t address, const std::vector<uint8_t>& value) {
  uint64_t bytes_written;
  bool res = WriteProcessMemory(handle_, reinterpret_cast<void*>(address), value.data(), value.size(), &bytes_written);

  return res && bytes_written == value.size();
}

char* WindowsImpl::allocMMapDisk(uint64_t size) {
  char* ret = static_cast<char*>(VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

  return ret;
}

};  // namespace

std::vector<ProcessInfoT> getProcTargets() {
  HANDLE hProcessSnap;
  PROCESSENTRY32 pe32;

  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  pe32.dwSize = sizeof(PROCESSENTRY32);

  if (!Process32First(hProcessSnap, &pe32)) {
    Log::Error("Failed to get processes");
    CloseHandle(hProcessSnap);  // apparently we are supposed to close SOME handles.
    return {};
  }

  std::vector<ProcessInfoT> Proccesses;
  do {
    Proccesses.push_back({static_cast<int32_t>(pe32.th32ProcessID), pe32.szExeFile, ""});
  } while (Process32Next(hProcessSnap, &pe32));

  CloseHandle(hProcessSnap);
  return Proccesses;
}

std::unique_ptr<IProcess> attach(int pid) { return std::make_unique<WindowsImpl>(pid); }

}  // namespace ProcessOS
