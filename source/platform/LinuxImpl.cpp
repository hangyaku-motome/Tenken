
#include "ActOS.h"
#include "LogW.h"
#include "types.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <sys/uio.h>
#include <vector>

namespace ActOS {
std::vector<ProcessInfoT> GetTargetProc();

namespace {
std::vector<int> ListPid();
std::string ReadFileString(std::string path);

class LinuxImpl : public IProcess {
  int pid_;

public:
  LinuxImpl(int pid) : pid_(pid) {}

  std::vector<MapInfoT> getRegions() override;
  std::vector<uint8_t> read(const uint64_t address,
                            const uint64_t ReadSize) override;
  bool write(const uint64_t address,
             const std::vector<uint8_t> &value) override;

}; // namespace LinuxImpl IProcess

std::vector<MapInfoT> LinuxImpl::getRegions() {
  std::ifstream maps;
  maps.open("/proc/" + std::to_string(pid_) + "/maps");
  if (!maps.is_open()) {
    Log::Error("Couldn't open maps!" + std::string(strerror(errno)));
    return {};
  }

  // For now I will hard code the fiters. we should add the choice to change
  // these.

  std::vector<MapInfoT> MapRegions;

  std::string MapsLine;

  while (getline(maps, MapsLine)) {
    std::istringstream SplitMapsLine(MapsLine);

    std::string MemoryAddresses, perms, name, uneeded;

    SplitMapsLine >> MemoryAddresses >> perms >> uneeded >> uneeded >>
        uneeded >> name;

    if (perms[0] == '-' || perms[1] == '-' || perms[2] == 'x' ||
        perms[3] == 's')
      continue;
    if (name.find("/lib/") != std::string::npos)
      continue;

    if (name.empty())
      name = "UNNAMED_REGION";

    if (MemoryAddresses.find("-") == std::string::npos) {
      printf("couldn't find \"-\" in maps\n");
      exit(1);
    }

    std::string StartStr = MemoryAddresses.substr(0, MemoryAddresses.find("-"));
    std::string EndStr = MemoryAddresses.substr(MemoryAddresses.find("-") + 1);

    uint64_t start = stoul(StartStr, nullptr, 16);
    uint64_t end = stoul(EndStr, nullptr, 16);

    MapInfoT TempMapReg = {start, end, name};

    MapRegions.push_back(TempMapReg);
  }
  return MapRegions;
};

std::vector<uint8_t> LinuxImpl::read(const uint64_t address,
                                     const uint64_t ReadSize) {
  std::vector<uint8_t> read_buf(ReadSize);

  struct iovec Receive;
  struct iovec WriteTo;

  Receive.iov_base = read_buf.data();
  Receive.iov_len = ReadSize;

  WriteTo.iov_base = reinterpret_cast<void *>(address);
  WriteTo.iov_len = ReadSize;

  int64_t read_amount = process_vm_readv(pid_, &Receive, 1, &WriteTo, 1, 0);

  if (read_amount == -1) {
    std::stringstream ss;
    ss << std::hex << address;
    // Some sort of counter for unreadable.
    return {};
  }

  return read_buf;
}

bool LinuxImpl::write(const uint64_t address,
                      const std::vector<uint8_t> &value) {
  struct iovec Receive;
  struct iovec WriteTo;

  Receive.iov_base = const_cast<unsigned char *>(value.data());
  Receive.iov_len = value.size();

  WriteTo.iov_base = reinterpret_cast<void *>(address);
  WriteTo.iov_len = value.size();
  int64_t write_amount = process_vm_writev(pid_, &Receive, 1, &WriteTo, 1, 0);

  // checking -1 would be unncessary if we check for value size but we should be
  // explicit about that failure condition.
  if (write_amount == -1 | write_amount != value.size()) {
    return false;
  }
  return true;
}

std::vector<int> ListPid() {
  std::vector<int> pidList;

  for (const auto &field : std::filesystem::directory_iterator("/proc")) {
    if (!field.is_directory())
      continue;

    int pid = atoi(field.path().filename().c_str());
    if (!pid)
      continue;

    pidList.push_back(pid);
  }

  return pidList;
}

std::string ReadFileString(std::string path) {
  std::ifstream PathStream(path);
  if (!PathStream)
    return "";
  std::stringstream readString;
  readString << PathStream.rdbuf();
  return readString.str();
}

}; // namespace
std::vector<ProcessInfoT> GetProcTargets() {
  std::vector<ProcessInfoT> Processes;

  for (int pid : ListPid()) {
    std::string comm, cmdline, uid;
    // TODO: Implement uid.

    std::string path = "/proc/" + std::to_string(pid) + "/";

    comm = ReadFileString(path + "comm");
    if (comm.empty())
      continue;
    comm.erase(comm.find("\n"));
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
    PushProcess.uid = 0; // later.
    PushProcess.FieldCmdline = cmdline;
    PushProcess.FieldComm = comm;

    Processes.push_back(PushProcess);
  }
  return Processes;
};
std::unique_ptr<IProcess> Attach(int pid) {
  return std::make_unique<LinuxImpl>(pid);
}
} // namespace ActOS
