#include "display.h"
#include "TargetPopUp.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstdlib>
#include <sys/types.h>

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWwindow *initalise_main() {
  glfwSetErrorCallback(glfw_error_callback);

  if (glfwInit() == 0)
    exit(1);

  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  float main_scale =
      ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

  GLFWwindow *window = glfwCreateWindow(static_cast<int32_t>(1280 * main_scale),
                                        static_cast<int32_t>(800 * main_scale),
                                        "Tenken", nullptr, nullptr);
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

void SetDefaultDisplay() {
  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
      ImGuiWindowFlags_NoBackground;

  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  ImGui::Begin("DockingWindow", nullptr, flags);
  ImGui::PopStyleVar(3);

  ImGuiID dockspaceID = ImGui::GetID("DockSpace");
  ImGui::DockSpace(dockspaceID, ImVec2(0, 0),
                   ImGuiDockNodeFlags_PassthruCentralNode);

  static bool first_launch = true;
  if (first_launch) {
    ImGui::DockBuilderRemoveNode(dockspaceID);
    ImGui::DockBuilderAddNode(dockspaceID,
                              ImGuiDockNodeFlags_PassthruCentralNode |
                                  ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

    ImGuiID main = dockspaceID;
    ImGuiID bottom =
        ImGui::DockBuilderSplitNode(main, ImGuiDir_Down, 0.3F, nullptr, &main);
    ImGuiID top_right =
        ImGui::DockBuilderSplitNode(main, ImGuiDir_Right, 0.3F, nullptr, &main);

    ImGuiID bottom_right = ImGui::DockBuilderSplitNode(bottom, ImGuiDir_Right,
                                                       0.5F, nullptr, &bottom);

    ImGui::DockBuilderDockWindow("Hits", main);
    ImGui::DockBuilderDockWindow("Log", bottom);
    ImGui::DockBuilderDockWindow("Search", top_right);
    ImGui::DockBuilderDockWindow("Favourite", bottom_right);

    ImGui::DockBuilderFinish(dockspaceID);

    first_launch = false;
  }
  ImGui::End();
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
