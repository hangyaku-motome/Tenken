#include <string>
#include <vector>

struct ProcessInfo {
  int pid = 0;
  std::string FieldComm = "";
  std::string FieldCmdline = "";
  int uid = 0;
};

std::vector<int> ListPid();

std::vector<ProcessInfo> ProcessScanner();