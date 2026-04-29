
#include "ActOS.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>
#include <string>

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

std::vector<ProcessInfoT> ActOS::GetTargets() {
  std::vector<ProcessInfoT> Processes;

  for (int pid : ListPid()) {
    std::string comm, cmdline, uid;
    // TODO: Implement uid.

    std::string path = "/proc/" + std::to_string(pid) + "/";

    comm = ReadFileString(path + "comm");
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