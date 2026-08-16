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

#include "Log.h"
#include "ProcessOS.h"
#include "types.h"

namespace ProcessOS {
std::vector<ProcessInfo> getTargetProc();

namespace {
std::vector<int> listPid();
std::string readFileString(const std::string& path);

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
  bool write(const std::vector<uint8_t>& value, uint64_t address) override;
  char* allocMMapDisk(uint64_t size) override;
  void unAllocMMapDisk(uint64_t address, uint64_t size) override;
  bool isAttached() override;

};  // namespace LinuxImpl IProcess

bool LinuxImpl::isAttached() { return pid_ != 0; }

void LinuxImpl::unAllocMMapDisk(uint64_t address, uint64_t size) { munmap(reinterpret_cast<void*>(address), size); }

// This should be dumb. JUST get regions. Nothing else. Filtering will happen above.
std::vector<MapInfo> LinuxImpl::getRegions() {
  std::ifstream map_stream;
  map_stream.open("/proc/" + std::to_string(pid_) + "/maps");
  if (!map_stream.is_open()) {
    Log::error("Couldn't open maps! %s", strerror(errno));
    return {};
  }
  std::vector<MapInfo> map_regions;
  std::string map_line;

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
  std::unordered_set<std::string> library_modules;

  while (getline(map_stream, map_line)) {
    std::istringstream SplitMapsLine(map_line);
    std::string name;
    std::string unneeded;
    SplitMapsLine >> unneeded >> unneeded >> unneeded >> unneeded >> unneeded >> name;
    library_modules.insert(name);
  }

  map_stream.clear();
  map_stream.seekg(0, std::ios::beg);

  while (getline(map_stream, map_line)) {
    std::istringstream split_maps_line(map_line);

    std::string memory_adr;
    std::string perms;
    std::string name;
    std::string unneeded;

    split_maps_line >> memory_adr >> perms >> unneeded >> unneeded >> unneeded >> name;
    if (memory_adr.find('-') == std::string::npos) {
      // wouldn't happen but...
      continue;
    }
    std::string start_str = memory_adr.substr(0, memory_adr.find('-'));
    std::string end_str = memory_adr.substr(memory_adr.find('-') + 1);

    uint64_t start = stoull(start_str, nullptr, 16);
    uint64_t end = stoull(end_str, nullptr, 16);

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
    } else if (library_modules.contains(name)) {
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

    MapInfo temp_map_reg = {name, start, end, type};
    map_regions.push_back(temp_map_reg);
  }

  return map_regions;
};

std::vector<uint8_t> LinuxImpl::read(const uint64_t address, const uint64_t read_size) {
  std::vector<uint8_t> read_buf(read_size);

  struct iovec receive{};
  struct iovec write_to{};

  receive.iov_base = read_buf.data();
  receive.iov_len = read_size;

  write_to.iov_base = reinterpret_cast<void*>(address);
  write_to.iov_len = read_size;

  int64_t read_amount = process_vm_readv(pid_, &receive, 1, &write_to, 1, 0);

  if (read_amount == -1) {
    return {};
  }
  read_buf.resize(static_cast<uint64_t>(read_amount));

  return read_buf;
}

bool LinuxImpl::write(const std::vector<uint8_t>& value, const uint64_t address) {
  struct iovec receive{};
  struct iovec write_to{};

  receive.iov_base = const_cast<unsigned char*>(value.data());
  receive.iov_len = value.size();

  write_to.iov_base = reinterpret_cast<void*>(address);
  write_to.iov_len = value.size();
  int64_t write_amount = process_vm_writev(pid_, &receive, 1, &write_to, 1, 0);

  // checking -1 would be unncessary if we check for value size but we should be
  // explicit about that failure condition.
  return write_amount != -1 && static_cast<uint64_t>(write_amount) == value.size();
}

std::vector<int> listPid() {
  std::vector<int> pid_list;

  for (const auto& field : std::filesystem::directory_iterator("/proc")) {
    if (!field.is_directory()) continue;

    int pid = atoi(field.path().filename().c_str());
    if (pid == 0) continue;

    pid_list.push_back(pid);
  }

  return pid_list;
}

std::string readFileString(const std::string& path) {
  std::ifstream path_stream(path);
  if (!path_stream) return "";
  std::stringstream read_string;
  read_string << path_stream.rdbuf();
  return read_string.str();
}

char* LinuxImpl::allocMMapDisk(uint64_t size) {
  uint64_t page_size = static_cast<uint64_t>(sysconf(_SC_PAGESIZE));

  uint64_t aligned_size = (size + page_size - 1) & ~(page_size - 1);

  uint64_t curr_offset = fileoffset_;
  fileoffset_ += aligned_size;

  if (ftruncate(fd_, static_cast<int64_t>(fileoffset_)) == -1) {
    Log::error("allocating disk space failed {}", strerror(errno));
    fileoffset_ -= aligned_size;
    return nullptr;
  }

  char* ptr = static_cast<char*>(
      mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, static_cast<int64_t>(curr_offset)));

  if (ptr == MAP_FAILED) {
    Log::error("mmap failed. {}", strerror(errno));
    return nullptr;
  }
  return ptr;
}

};  // namespace

std::vector<ProcessInfo> getProcTargets() {
  std::vector<ProcessInfo> Processes;

  for (int pid : listPid()) {
    std::string name;
    std::string cmdline;

    std::string path = "/proc/" + std::to_string(pid) + "/";

    name = readFileString(path + "comm");
    if (name.empty()) continue;
    name.erase(name.find('\n'));
    cmdline = readFileString(path + "cmdline");
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
