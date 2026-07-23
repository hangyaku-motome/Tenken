#include <fcntl.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>

#include "LogW.h"
#include "ProcessOS.h"
#include "types.h"

namespace ProcessOS {
std::vector<ProcessInfo> GetTargetProc();

namespace {
std::vector<int> ListPid();
std::string ReadFileString(const std::string& path);

class LinuxImpl : public IProcess {
  int pid_;
  int fd_ = -1;
  uint64_t fileoffset_ = 0;

public:
  LinuxImpl(int pid)
      : pid_(pid) {
    std::string path = "/var/tmp/tenken_mmap_" + std::to_string(getpid());
    fd_ = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
  }

  ~LinuxImpl() {
    if (fd_) close(fd_);
  }

  std::vector<MapInfo> getRegions() override;
  std::vector<uint8_t> read(uint64_t address, uint64_t ReadSize) override;
  bool write(uint64_t address, const std::vector<uint8_t>& value) override;
  char* allocMMapDisk(uint64_t size) override;
  void unAllocMMapDisk(uint64_t address, uint64_t size) override;
  bool isAttached() override;

};  // namespace LinuxImpl IProcess

bool LinuxImpl::isAttached() { return pid_ != 0; }

void LinuxImpl::unAllocMMapDisk(uint64_t address, uint64_t size) { munmap(reinterpret_cast<void*>(address), size); }

// This should be dumb. JUST get regions. Nothing else. Filtering will happen above.
std::vector<MapInfo> LinuxImpl::getRegions() {
  std::ifstream mapsStream;
  mapsStream.open("/proc/" + std::to_string(pid_) + "/maps");
  if (!mapsStream.is_open()) {
    Log::error("Couldn't open maps!" + std::string(strerror(errno)));
    return {};
  }
  std::vector<MapInfo> MapRegions;
  std::string MapsLine;

  constexpr int32_t MAX_PATH = 4096;
  char exec_name[MAX_PATH];

  int64_t len = readlink(std::string("/proc/" + std::to_string(pid_) + "/exe").c_str(), exec_name, MAX_PATH);

  if (len > 0) {
    exec_name[len] = '\0';
  } else {
    exec_name[len] = '\0';
    Log::error("Failed to read /proc/pid/exe");
  }

  // okay I decided I will first do a scan of all executable files, and put them all here excluding the main exec name.
  std::unordered_set<std::string> libraries;

  while (getline(mapsStream, MapsLine)) {
    std::istringstream SplitMapsLine(MapsLine);
    std::string name;
    std::string unneeded;
    SplitMapsLine >> unneeded >> unneeded >> unneeded >> unneeded >> unneeded >> name;
    libraries.insert(name);
  }

  mapsStream.clear();
  mapsStream.seekg(0, std::ios::beg);

  while (getline(mapsStream, MapsLine)) {
    std::istringstream SplitMapsLine(MapsLine);

    std::string MemoryAddresses;
    std::string perms;
    std::string name;
    std::string unneeded;

    SplitMapsLine >> MemoryAddresses >> perms >> unneeded >> unneeded >> unneeded >> name;
    if (MemoryAddresses.find('-') == std::string::npos) {
      // wouldn't happen but...
      continue;
    }
    std::string StartStr = MemoryAddresses.substr(0, MemoryAddresses.find('-'));
    std::string EndStr = MemoryAddresses.substr(MemoryAddresses.find('-') + 1);

    uint64_t start = stoull(StartStr, nullptr, 16);
    uint64_t end = stoull(EndStr, nullptr, 16);

    MapType type;

    // the only filter I will hard code.
    if (name.find("anon_inode:") != std::string::npos) continue;

    if (name == exec_name) {
      if (perms.find('x') != std::string::npos)
        type = MapType::mainExecCode;
      else if (perms.find('w') != std::string::npos)
        type = MapType::mainExecData;
      else
        type = MapType::mainExecConst;
    } else if (name.empty()) {
      type = MapType::anon;
      name = "UNNAMED_REGION";
    } else if (name == "[stack]") {
      type = MapType::stack;
    } else if (name == "[heap]") {
      type = MapType::heap;
    } else if (name == "[vvar]" || name == "[vvar_vlock]" || name == "[vdso]" || name == "[vsyscall]") {
      type = MapType::kernelPages;
    } else if (libraries.contains(name)) {
      if (perms.find('x') != std::string::npos)
        type = MapType::sharedLibCode;
      else if (perms.find('w') != std::string::npos)
        type = MapType::sharedLibData;
      else
        type = MapType::sharedLibConst;
    } else if (perms.find('r') == std::string::npos) {
      type = MapType::unreadable;
    } else
      type = MapType::unset;

    MapInfo TempMapReg = {name, start, end, type};
    MapRegions.push_back(TempMapReg);
  }

  return MapRegions;
};

std::vector<uint8_t> LinuxImpl::read(const uint64_t address, const uint64_t ReadSize) {
  std::vector<uint8_t> read_buf(ReadSize);

  struct iovec Receive{};
  struct iovec WriteTo{};

  Receive.iov_base = read_buf.data();
  Receive.iov_len = ReadSize;

  WriteTo.iov_base = reinterpret_cast<void*>(address);
  WriteTo.iov_len = ReadSize;

  int64_t read_amount = process_vm_readv(pid_, &Receive, 1, &WriteTo, 1, 0);

  if (read_amount == -1) {
    return {};
  }
  read_buf.resize(static_cast<uint64_t>(read_amount));

  return read_buf;
}

bool LinuxImpl::write(const uint64_t address, const std::vector<uint8_t>& value) {
  struct iovec Receive{};
  struct iovec WriteTo{};

  Receive.iov_base = const_cast<unsigned char*>(value.data());
  Receive.iov_len = value.size();

  WriteTo.iov_base = reinterpret_cast<void*>(address);
  WriteTo.iov_len = value.size();
  int64_t write_amount = process_vm_writev(pid_, &Receive, 1, &WriteTo, 1, 0);

  // checking -1 would be unncessary if we check for value size but we should be
  // explicit about that failure condition.
  return write_amount != -1 && static_cast<uint64_t>(write_amount) == value.size();
}

std::vector<int> ListPid() {
  std::vector<int> pidList;

  for (const auto& field : std::filesystem::directory_iterator("/proc")) {
    if (!field.is_directory()) continue;

    int pid = atoi(field.path().filename().c_str());
    if (pid == 0) continue;

    pidList.push_back(pid);
  }

  return pidList;
}

std::string ReadFileString(const std::string& path) {
  std::ifstream PathStream(path);
  if (!PathStream) return "";
  std::stringstream readString;
  readString << PathStream.rdbuf();
  return readString.str();
}

char* LinuxImpl::allocMMapDisk(uint64_t size) {
  uint64_t page_size = static_cast<uint64_t>(sysconf(_SC_PAGESIZE));

  uint64_t aligned_size = (size + page_size - 1) & ~(page_size - 1);

  uint64_t curr_offset = fileoffset_;
  fileoffset_ += aligned_size;

  if (ftruncate(fd_, static_cast<int64_t>(fileoffset_)) == -1) {
    Log::error("allocating disk space failed" + std::string(strerror(errno)));
    fileoffset_ -= aligned_size;
    return nullptr;
  }

  char* ptr = static_cast<char*>(
      mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, static_cast<int64_t>(curr_offset)));

  if (ptr == MAP_FAILED) {
    Log::error("mmap failed." + std::string(strerror(errno)));
    return nullptr;
  }
  return ptr;
}

};  // namespace

std::vector<ProcessInfo> getProcTargets() {
  std::vector<ProcessInfo> Processes;

  for (int pid : ListPid()) {
    std::string name;
    std::string cmdline;

    std::string path = "/proc/" + std::to_string(pid) + "/";

    name = ReadFileString(path + "comm");
    if (name.empty()) continue;
    name.erase(name.find('\n'));
    cmdline = ReadFileString(path + "cmdline");
    if (cmdline.empty()) {
      continue;
      // It seems some have the cmdline as "systemd-userwork: waiting...".
      // Most likely also irrelevant. Will filter them later. I'll need to
      // clarify what the state of cmdline means. Right now I will assume
      // empty means -> Irrelevant.
    }

    ProcessInfo PushProcess;

    PushProcess.pid = pid;
    PushProcess.cmdline = cmdline;
    PushProcess.name = name;

    Processes.push_back(PushProcess);
  }
  return Processes;
};

std::unique_ptr<IProcess> attach(int pid) { return std::make_unique<LinuxImpl>(pid); }
}  // namespace ProcessOS
