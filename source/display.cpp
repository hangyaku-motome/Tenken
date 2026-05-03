#include "display.h"
#include "LogW.h"
#include "TargetPopUp.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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

void SetDisplayInfo(GLFWwindow *window, DisplayInfoT &DisplayInfo) {
  int tempdisplay_w, tempdisplay_h;

  glfwGetFramebufferSize(window, &tempdisplay_w, &tempdisplay_h);
  if (DisplayInfo.display_h == tempdisplay_h &&
      DisplayInfo.display_w == tempdisplay_w)
    return;
  DisplayInfo.TopMenuHeight = ImGui::GetFrameHeight();

  DisplayInfo.display_h = tempdisplay_h;
  DisplayInfo.display_w = tempdisplay_w;

  DisplayInfo.Hit.W = DisplayInfo.display_w / 2.0 * 1.3;
  DisplayInfo.Hit.H = DisplayInfo.display_h / 2.0 * 1.1;
  DisplayInfo.Hit.XPos = 0;
  DisplayInfo.Hit.YPos = DisplayInfo.TopMenuHeight;

  DisplayInfo.Log.W = DisplayInfo.display_w;
  DisplayInfo.Log.H =
      DisplayInfo.display_h - DisplayInfo.Hit.H - DisplayInfo.TopMenuHeight;
  DisplayInfo.Log.XPos = 0;
  DisplayInfo.Log.YPos = DisplayInfo.Hit.H + DisplayInfo.TopMenuHeight;

  DisplayInfo.Search.W = DisplayInfo.display_w - DisplayInfo.Hit.W;
  DisplayInfo.Search.H = DisplayInfo.Hit.H;
  DisplayInfo.Search.XPos = DisplayInfo.Hit.W;
  DisplayInfo.Search.YPos = DisplayInfo.TopMenuHeight;
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

template <typename T> T readAs(const std::vector<uint8_t> &buffer) {
  T This{};

  if (buffer.size() >= sizeof(T))
    memcpy((void *)&This, buffer.data(), sizeof(This));

  return This;
}

// maybe add unsigned/signed to TargetType itself?
std::string HitToStr(const std::vector<uint8_t> &Bytes,
                     TargetInfoT TargetInfo) {

  if (TargetInfo.IsUnsigned) {
    switch (TargetInfo.TargetType) {
    case TargetTypeT::Int8:
      return std::to_string(readAs<int8_t>(Bytes));
    case TargetTypeT::Int16:
      return std::to_string(readAs<int16_t>(Bytes));
    case TargetTypeT::Int32:
      return std::to_string(readAs<int32_t>(Bytes));
    case TargetTypeT::Int64:
      return std::to_string(readAs<int64_t>(Bytes));
    case TargetTypeT::Float:
      return std::to_string(readAs<float>(Bytes));
    case TargetTypeT::Double:
      return std::to_string(readAs<double>(Bytes));
    case TargetTypeT::String:
      return readAs<std::string>(Bytes);
    default:
      Log::Error("Why is TargetType undefined in HitToStr?? (TargetType: " +
                 std::to_string((int)TargetInfo.TargetType));
      return {};
    }
  } else {
    switch (TargetInfo.TargetType) {
    case TargetTypeT::Int8:
      return std::to_string(readAs<uint8_t>(Bytes));
    case TargetTypeT::Int16:
      return std::to_string(readAs<uint16_t>(Bytes));
    case TargetTypeT::Int32:
      return std::to_string(readAs<uint32_t>(Bytes));
    case TargetTypeT::Int64:
      return std::to_string(readAs<uint64_t>(Bytes));
    default:
      Log::Error("Why is TargetType undefined in HitToStr?? (TargetType: " +
                 std::to_string((int)TargetInfo.TargetType));
      return {};
    }
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