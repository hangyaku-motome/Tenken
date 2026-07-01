#include "display.h"

#include <GLFW/glfw3.h>
#include <sys/types.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
#include <type_traits>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "types.h"
#include "utils.h"

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWwindow* initalise_imgui(const std::filesystem::path& ImGuiInitPath) {
  glfwSetErrorCallback(glfw_error_callback);

  if (glfwInit() == 0) exit(1);

  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

  GLFWwindow* window = glfwCreateWindow(
      static_cast<int32_t>(1280 * main_scale), static_cast<int32_t>(800 * main_scale), "Tenken", nullptr, nullptr);
  if (window == nullptr) exit(1);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  if (!ImGuiInitPath.empty()) {
    std::filesystem::create_directories(ImGuiInitPath.parent_path());
    io.IniFilename = ImGuiInitPath.c_str();
  }
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);
  style.FontScaleDpi = main_scale;

  ImGui_ImplGlfw_InitForOpenGL(window, true);

  ImGui_ImplOpenGL3_Init(glsl_version);

  return window;
}

void exit_main(GLFWwindow* window) {
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

void end_frame(int display_w, int display_h, ImVec4 clear_color, GLFWwindow* window) {
  ImGui::Render();
  glViewport(0, 0, display_w, display_h);
  glClearColor(
      clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window);
}

void SetDefaultDisplay() {
  ImGuiWindowFlags flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                           ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
                           ImGuiWindowFlags_NoBackground;

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  ImGui::Begin("DockingWindow", nullptr, flags);
  ImGui::PopStyleVar(3);

  ImGuiID dockspaceID = ImGui::GetID("DockSpace");

  if (ImGui::DockBuilderGetNode(dockspaceID) == nullptr) {
    ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

    ImGuiID main = dockspaceID;
    ImGuiID bottom = ImGui::DockBuilderSplitNode(main, ImGuiDir_Down, 0.3F, nullptr, &main);
    ImGuiID top_right = ImGui::DockBuilderSplitNode(main, ImGuiDir_Right, 0.3F, nullptr, &main);

    ImGuiID bottom_right = ImGui::DockBuilderSplitNode(bottom, ImGuiDir_Right, 0.5F, nullptr, &bottom);

    ImGui::DockBuilderDockWindow("Hits", main);
    ImGui::DockBuilderDockWindow("Log", bottom);
    ImGui::DockBuilderDockWindow("Search", top_right);
    ImGui::DockBuilderDockWindow("Favourite", bottom_right);
    ImGui::DockBuilderDockWindow("Hex", bottom);
    ImGui::DockBuilderDockWindow("Inspector", top_right);

    ImGui::DockBuilderFinish(dockspaceID);
  }
  ImGui::DockSpace(dockspaceID, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

  ImGui::End();
}

std::string MainMenuBarCycle(
    bool& TargetPUpClicked, bool& MapPUpClicked, bool& LogWEnabled, bool& HexWEnabled, bool& DataInspectorWEnabled) {
  if (!ImGui::BeginMainMenuBar()) return "";

  std::string doAction;

  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("New Target")) TargetPUpClicked = true;

    if (ImGui::MenuItem("Save")) doAction = "Save";
    if (ImGui::MenuItem("Load")) doAction = "Load";

    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Utils")) {
    if (ImGui::MenuItem("View Regions")) MapPUpClicked = true;

    if (ImGui::MenuItem("Toggle Log window.", nullptr, LogWEnabled, true)) LogWEnabled = !LogWEnabled;

    if (ImGui::MenuItem("Toggle Hex window.", nullptr, HexWEnabled, true)) HexWEnabled = !HexWEnabled;

    if (ImGui::MenuItem("Toggle Data Inspector window.", nullptr, DataInspectorWEnabled, true))
      DataInspectorWEnabled = !DataInspectorWEnabled;

    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();

  return doAction;
}

template <typename out, typename T> std::vector<uint8_t> ValtoData(const T& val) {
  std::vector<uint8_t> data(sizeof(out));
  memcpy(data.data(), &val, sizeof(out));
  return data;
}

// this is a mess. I should rewrite this.
bool GetTargetValue(const TargetTypeT TargetType, std::vector<uint8_t>& write_to, ImGuiInputTextFlags flags) {
  if (TargetType == TargetTypeT::Invalid) return false;

  std::string tempbuf;
  return dispatchType(TargetType, [&]<typename T>() -> bool {
    tempbuf = dataToStr<T>(write_to);

    if (!ImGui::InputText("##value", &tempbuf, flags)) {
      if (tempbuf.empty()) write_to.clear();
      return false;
    }

    if constexpr (std::is_same_v<T, std::string>) {
      write_to.assign(tempbuf.begin(), tempbuf.end());
      return true;
    } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
      std::istringstream stream(tempbuf);
      std::string token;

      std::vector<uint8_t> new_bytes;

      while (stream >> token) {
        try {
          new_bytes.push_back(static_cast<uint8_t>(std::stoi(token, nullptr, 16)));
        } catch (...) {
          write_to.clear();
          return false;
        }
      }
      if (new_bytes.empty()) {
        write_to.clear();
        return false;
      }
      write_to = new_bytes;
      return true;
    } else {
      try {
        T value;
        if constexpr (std::is_floating_point_v<T>)
          value = static_cast<T>(std::stod(tempbuf));
        else if constexpr (std::is_unsigned_v<T>)
          value = static_cast<T>(std::stoull(tempbuf));
        else
          value = static_cast<T>(std::stoll(tempbuf));

        write_to = ValtoData<T>(value);
        return true;
      } catch (...) {
        write_to.clear();
        return false;
      }
    }
  });
}

void printData(const std::vector<uint8_t>& data, TargetTypeT TargetType) {
  if (data.empty() || TargetType == TargetTypeT::Invalid) return;

  std::string print_str = dispatchType(TargetType, [&]<typename T>() { return dataToStr<T>(data); });
  ImGui::TextUnformatted(print_str.c_str());
}
