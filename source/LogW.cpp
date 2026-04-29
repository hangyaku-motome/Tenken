#include "LogW.h"
#include "imgui.h"
#include "types.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <string>

std::string LogText;

void LogW::InitW(WindowInfoT LogsWindow, ImGuiWindowFlags Flags) {
  ImGui::SetNextWindowPos(ImVec2(LogsWindow.XPos, LogsWindow.YPos));
  ImGui::SetNextWindowSize(ImVec2(LogsWindow.W, LogsWindow.H));
  ImGui::Begin("Log", nullptr, Flags);
}

void LogW::EndW() { ImGui::End(); }

void LogW::UpdateLog(LogEventsT Events) {
  if (Events.ChosenProc.pid) {
    std::stringstream tempss;
    tempss << "...Chosen PID: " << Events.ChosenProc.pid
           << "   Target Comm:" << Events.ChosenProc.FieldComm
           << "   Target CmdLine:" << Events.ChosenProc.FieldCmdline << "\n";
    LogText += tempss.str();
    std::cout << LogText;
  }
  if (Events.ProcCount) {
    std::stringstream tempss;
    tempss << "...Found PID count: " << Events.ProcCount << "\n";
    LogText += tempss.str();
  }
}

void LogW::CycleW(WindowInfoT LogsWindow, ImGuiWindowFlags Flags,
                  LogEventsT Events) {
  InitW(LogsWindow, Flags);
  UpdateLog(Events);
  ImGui::TextUnformatted(LogText.c_str(), LogText.end().base());
  EndW();
}
