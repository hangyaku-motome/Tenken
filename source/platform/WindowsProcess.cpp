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

#include "Log.h"
#include "ProcessOS.h"
#include "types.h"

// I hateee windows I ahtee widwos I hate widnows I hate ahteahtahahtehatewindwos I hateeeeeeeeeeeeeeeeeeeeeeeeeeeee
// This entire thing is as reliable as undefined behaviour

namespace ProcessOS {
std::vector<ProcessInfo> getProcesses();

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

  std::vector<MapInfo> getRegions() override;
  std::vector<uint8_t> read(uint64_t address, uint64_t ReadSize) override;
  bool write(const std::vector<uint8_t>& value, uint64_t address) override;
  char* allocMMapDisk(uint64_t size) override;
  void unAllocMMapDisk(uint64_t address, uint64_t size) override;
  bool isAttached() override;

};  // namespace WindowsImpl IProcess

std::string wideToUtf8(const wchar_t* w) {
  if (!w) return {};
  int size = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
  if (size <= 1) return {};
  std::string out(size - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, w, -1, out.data(), size, nullptr, nullptr);
  return out;
}

bool WindowsImpl::isAttached() { return handle_ != nullptr; }

void WindowsImpl::unAllocMMapDisk(uint64_t address, uint64_t size) {
  VirtualFree(reinterpret_cast<void*>(address), 0, MEM_RELEASE);
}

std::vector<MapInfo> WindowsImpl::getRegions() {
  std::vector<MapInfo> maps;

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

    MapInfo region;
    region.start = reinterpret_cast<uint64_t>(region_info.BaseAddress);
    region.end = region.start + region_info.RegionSize;

    const bool exec =
        region_info.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
    const bool write =
        region_info.Protect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);

    switch (region_info.Type) {
      case MEM_IMAGE: {
        auto it = module_map.find(reinterpret_cast<uint64_t>(region_info.AllocationBase));
        const bool is_main = (it != module_map.end()) && it->second.is_main;
        if (it != module_map.end()) {
          region.name = wideToUtf8(it->second.path.c_str());
        }
        if (region.name.empty()) region.name = "UNNAMED_MODULE";

        if (exec) {
          region.type = is_main ? MapType::mainExecCode : MapType::sharedLibCode;
        } else if (write) {
          region.type = is_main ? MapType::mainExecData : MapType::sharedLibData;
        } else {
          region.type = is_main ? MapType::mainExecConst : MapType::sharedLibConst;
        }
        break;
      }

      case MEM_MAPPED: {
        region.type = MapType::anon;
        wchar_t fname_wstr[MAX_PATH];
        if (K32GetMappedFileNameW(handle_, region_info.BaseAddress, fname_wstr, MAX_PATH)) {
          region.name = wideToUtf8(fname_wstr);
        }
        if (region.name.empty()) region.name = "UNNAMED_REGION";
        break;
      }

      case MEM_PRIVATE: {
        // TODO: no stack or heap
        region.type = MapType::anon;
        region.name = "UNNAMED_REGION";
        break;
      }

      default: {
        region.type = MapType::anon;
        region.name = "UNNAMED_REGION";
        break;
      }
    }

    maps.push_back(std::move(region));
    address = next_address;
  }

  return maps;
}

std::vector<uint8_t> WindowsImpl::read(uint64_t address, uint64_t read_size) {
  std::vector<uint8_t> read_bytes(read_size);

  bool res = ReadProcessMemory(handle_, reinterpret_cast<void*>(address), read_bytes.data(), read_size, NULL);

  if (res == 0) {
    return {};
  }
  return read_bytes;
}

bool WindowsImpl::write(const std::vector<uint8_t>& value, uint64_t address) {
  uint64_t bytes_written;
  bool res = WriteProcessMemory(handle_, reinterpret_cast<void*>(address), value.data(), value.size(), &bytes_written);

  return res && bytes_written == value.size();
}

char* WindowsImpl::allocMMapDisk(uint64_t size) {
  char* ret = static_cast<char*>(VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

  return ret;
}

};  // namespace

std::vector<ProcessInfo> getProcesses() {
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    Log::error("CreateToolhelp32Snapshot failed");
    return {};
  }

  PROCESSENTRY32W pe32;
  pe32.dwSize = sizeof(pe32);

  if (!Process32FirstW(snapshot, &pe32)) {
    Log::error("Process32FirstW failed");
    CloseHandle(snapshot);
    return {};
  }

  std::vector<ProcessInfo> processes;
  do {
    processes.push_back(
        {.name = wideToUtf8(pe32.szExeFile), .cmdline = "", .pid = static_cast<int32_t>(pe32.th32ProcessID)});
  } while (Process32NextW(snapshot, &pe32));

  CloseHandle(snapshot);
  return processes;
}

std::unique_ptr<IProcess> attach(int pid) { return std::make_unique<WindowsImpl>(pid); }

}  // namespace ProcessOS
