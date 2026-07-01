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
#include <memory>
#include <sstream>
#include <string>

#include "LogW.h"
#include "types.h"
#include "ProcessOS.h"

namespace ProcessOS {
std::vector<ProcessInfoT> GetTargetProc();

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
    pid_ = pid;
    std::string path = "/tmp/tenken_mmap_" + std::to_string(getpid());
    fd_ = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
  }

  ~LinuxImpl() {
    if (fd_) close(fd_);
  }

  std::vector<MapInfoT> getRegions() override;
  std::vector<uint8_t> read(uint64_t address, uint64_t ReadSize) override;
  bool write(uint64_t address, const std::vector<uint8_t>& value) override;
  char* allocMMapDisk(uint64_t size) override;
  void unAllocMMapDisk(uint64_t address, uint64_t size) override;

};  // namespace LinuxImpl IProcess

void LinuxImpl::unAllocMMapDisk(uint64_t address, uint64_t size) { munmap(reinterpret_cast<void*>(address), size); }

std::vector<MapInfoT> LinuxImpl::getRegions() {
  std::ifstream maps;
  maps.open("/proc/" + std::to_string(pid_) + "/maps");
  if (!maps.is_open()) {
    Log::Error("Couldn't open maps!" + std::string(strerror(errno)));
    return {};
  }

  // For now I will hard code the fiters. I should add the choice to change
  // these.

  std::vector<MapInfoT> MapRegions;

  std::string MapsLine;

  while (getline(maps, MapsLine)) {
    std::istringstream SplitMapsLine(MapsLine);

    std::string MemoryAddresses;
    std::string perms;
    std::string name;
    std::string uneeded;

    SplitMapsLine >> MemoryAddresses >> perms >> uneeded >> uneeded >> uneeded >> name;

    if (perms[0] == '-' || perms[1] == '-' || perms[2] == 'x' || perms[3] == 's') continue;
    if (name.find("/lib/") != std::string::npos) continue;

    if (name.empty()) name = "UNNAMED_REGION";

    if (MemoryAddresses.find('-') == std::string::npos) {
      // wouldn't happen but...
      continue;
    }

    std::string StartStr = MemoryAddresses.substr(0, MemoryAddresses.find('-'));
    std::string EndStr = MemoryAddresses.substr(MemoryAddresses.find('-') + 1);

    uint64_t start = stoull(StartStr, nullptr, 16);
    uint64_t end = stoull(EndStr, nullptr, 16);

    MapInfoT TempMapReg = {start, end, name};

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
  read_buf.resize(read_amount);

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
  uint64_t pagesize = sysconf(_SC_PAGESIZE);

  uint64_t alignedsize = (size + pagesize - 1) & ~(pagesize - 1);

  uint64_t curr_offset = fileoffset_;
  fileoffset_ += alignedsize;

  ftruncate(fd_, static_cast<int64_t>(fileoffset_));

  char* ptr = static_cast<char*>(
      mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, static_cast<int64_t>(curr_offset)));

  if (ptr == MAP_FAILED) {
    Log::Error("mmap failed." + std::string(strerror(errno)));
    return nullptr;
  }
  return ptr;
}

};  // namespace

std::vector<ProcessInfoT> getProcTargets() {
  std::vector<ProcessInfoT> Processes;

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

    ProcessInfoT PushProcess;

    PushProcess.pid = pid;
    PushProcess.cmdline = cmdline;
    PushProcess.name = name;

    Processes.push_back(PushProcess);
  }
  return Processes;
};

std::unique_ptr<IProcess> attach(int pid) { return std::make_unique<LinuxImpl>(pid); }
}  // namespace ActOS
