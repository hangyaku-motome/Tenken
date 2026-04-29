#pragma once

#include "../types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

// Does private and public even do anything here? I'm not sure how it behaves.

// Unimplemented for now cause I'm getting confused on how to make it work for
// now. class LinuxImpl : public ActOS { private:
//   std::vector<int> ListPid();
//   std::string ReadFileString(std::string path);

// public:
// };

std::vector<int> ListPid();
std::string ReadFileString(std::string path);
std::vector<ProcessInfo> GetTargets();