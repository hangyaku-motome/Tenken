#include "display.h"
#include "HitsW.h"
#include "LogW.h"
#include "SearchW.h"
#include "TargetPopUp.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "types.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstdlib>
#include <vector>

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWwindow *initalise_main() {
  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit())
    exit(1);

  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  float main_scale =
      ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

  GLFWwindow *window =
      glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale),
                       "My_MemScanner", nullptr, nullptr);
  if (window == nullptr)
    exit(1);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);
  style.FontScaleDpi = main_scale;

  ImGui_ImplGlfw_InitForOpenGL(window, true);

  ImGui_ImplOpenGL3_Init(glsl_version);

  return window;
}

void exit_main(GLFWwindow *window) {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}

void start_frame() {
  glfwPollEvents();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void end_frame(int display_w, int display_h, ImVec4 clear_color,
               GLFWwindow *window) {
  ImGui::Render();
  glViewport(0, 0, display_w, display_h);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
               clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window);
}

void SetDisplayInfo(GLFWwindow *window, DisplayInfoT DisplayInfo,
                    ImGuiWindowFlags flagsWindowDefault, LogW &LogObj,
                    SearchW &SearchObj, HitsW &HitObj) {
  int tempdisplay_w, tempdisplay_h;

  glfwGetFramebufferSize(window, &tempdisplay_w, &tempdisplay_h);
  if (DisplayInfo.display_h == tempdisplay_h &&
      DisplayInfo.display_w == tempdisplay_w)
    return;
  DisplayInfo.TopMenuHeight = ImGui::GetFrameHeight();

  DisplayInfo.display_h = tempdisplay_h;
  DisplayInfo.display_w = tempdisplay_w;

  HitObj.Window.W = DisplayInfo.display_w / 2.0 * 1.3;
  HitObj.Window.H = DisplayInfo.display_h / 2.0 * 1.1;
  HitObj.Window.XPos = 0;
  HitObj.Window.YPos = DisplayInfo.TopMenuHeight;
  HitObj.Window.flags = flagsWindowDefault;

  LogObj.Window.W = DisplayInfo.display_w;
  LogObj.Window.H =
      DisplayInfo.display_h - HitObj.Window.H - DisplayInfo.TopMenuHeight;
  LogObj.Window.XPos = 0;
  LogObj.Window.YPos = HitObj.Window.H + DisplayInfo.TopMenuHeight;
  LogObj.Window.flags = flagsWindowDefault;

  SearchObj.Window.W = DisplayInfo.display_w - HitObj.Window.W;
  SearchObj.Window.H = HitObj.Window.H;
  SearchObj.Window.XPos = HitObj.Window.W;
  SearchObj.Window.YPos = DisplayInfo.TopMenuHeight;
  SearchObj.Window.flags = flagsWindowDefault;
}

void MainMenuBarCycle(TargetPopUp &TargetPUp) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New Target")) {
        TargetPUp.IsClicked = true;
      }
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}

std::string BytesToHex(const std::vector<uint8_t> &Data) {
  char static constexpr hex[] = "0123456789ABCDEF";
  std::string ReturnStr;
  ReturnStr.reserve(Data.size() * 3);
  for (int i = 0; i < Data.size(); ++i) {
    if (i > 0)
      ReturnStr += " ";
    ReturnStr += hex[Data[i] >> 4];
    ReturnStr += hex[Data[i] & 0x0F];
  }
  return ReturnStr;
}