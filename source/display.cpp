#include "display.h"
#include "LogW.h"
#include "TargetPopUp.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "types.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
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
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
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
        ImGui::DockBuilderSplitNode(main, ImGuiDir_Down, 0.3f, nullptr, &main);
    ImGuiID top_right =
        ImGui::DockBuilderSplitNode(main, ImGuiDir_Right, 0.3f, nullptr, &main);

    ImGuiID bottom_right = ImGui::DockBuilderSplitNode(bottom, ImGuiDir_Right,
                                                       0.5f, nullptr, &bottom);

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

template <typename Out, typename In>
void WriteVal(const In val, std::vector<uint8_t> &write_to) {
  Out write;
  write = static_cast<Out>(val);
  write_to.resize(sizeof(Out));
  memcpy(write_to.data(), &write, sizeof(write));
}

bool GetTargetValue(const TargetTypeT TargetType,
                    std::vector<uint8_t> &write_to, ImGuiInputTextFlags flags) {

  if (TargetType == TargetTypeT::Invalid)
    return false;

  std::vector<uint8_t> tempbuf(64);
  if (!ImGui::InputText("##value", (char *)(tempbuf.data()), tempbuf.size(),
                        flags))
    return false;

  char *end;

  uint64_t u_val = strtoull((char *)(tempbuf.data()), &end, 10);
  bool uval_ok = (*end == '\0' || end != (char *)tempbuf.data());

  int64_t s_val = strtoll((char *)(tempbuf.data()), &end, 10);
  bool sval_ok = (*end == '\0' || end != (char *)tempbuf.data());

  switch (TargetType) {
  case TargetTypeT::uInt8:
    if (!(u_val <= UINT8_MAX) || !uval_ok)
      return false;
    WriteVal<uint8_t>(u_val, write_to);
    return true;
  case TargetTypeT::uInt16:
    if (!(u_val <= UINT16_MAX) || !uval_ok)
      return false;
    WriteVal<uint16_t>(u_val, write_to);
    return true;
  case TargetTypeT::uInt32:
    if (!(u_val <= UINT32_MAX) || !uval_ok)
      return false;
    WriteVal<uint32_t>(u_val, write_to);
    return true;
  case TargetTypeT::uInt64:
    if (!uval_ok)
      return false;
    WriteVal<uint64_t>(u_val, write_to);
    return true;
  case TargetTypeT::Int8:
    if (!(s_val <= INT8_MAX) || !(INT8_MIN <= s_val) || !sval_ok)
      return false;
    WriteVal<int8_t>(s_val, write_to);
    return true;
  case TargetTypeT::Int16:
    if (!(s_val <= INT16_MAX) || !(INT16_MIN <= s_val) || !sval_ok)
      return false;
    WriteVal<int16_t>(s_val, write_to);
    return true;
  case TargetTypeT::Int32:
    if (!(s_val <= INT32_MAX) || !(INT32_MIN <= s_val) || !sval_ok)
      return false;
    WriteVal<int32_t>(s_val, write_to);
    return true;
  case TargetTypeT::Int64:
    if (!sval_ok)
      return false;
    WriteVal<int64_t>(s_val, write_to);
    return true;
  case TargetTypeT::Float: {
    float float_value = strtof((char *)(tempbuf.data()), &end);
    if (!(*end == '\0' || end != (char *)tempbuf.data()))
      return false;
    WriteVal<float>(float_value, write_to);
    return true;
  }
  case TargetTypeT::Double: {
    double double_value = strtof((char *)(tempbuf.data()), &end);
    if (!(*end == '\0' || end != (char *)tempbuf.data()))
      return false;
    WriteVal<double>(double_value, write_to);
    return true;
  }
  case TargetTypeT::String: {
    auto null_ter = std::find(tempbuf.begin(), tempbuf.end(), '\0');
    write_to.resize(std::distance(tempbuf.begin(), null_ter));
    memcpy(write_to.data(), tempbuf.data(), write_to.size());
    return true;
  }
  default:
  case TargetTypeT::Invalid:
    Log::Error("Why did get target value recieve invalid target type?");
    return false;
  }
}

template <typename T> T readAs(const std::vector<uint8_t> &buffer) {
  T This{};

  static_assert(std::is_arithmetic_v<T>,
                "CompareValues only works with numeric types");

  if (buffer.size() >= sizeof(T))
    memcpy(&This, buffer.data(), sizeof(This));

  return This;
}

// for some reason this kind of feels redundant. will check on the exact logic
// later.
// (Later in question...) Ohh wait what about a function that returns a type
// based on TargetInfoT, and then with that return we can implement a lambda?
// I'll look into that later.
// Yeah IDK what I meant with that up there anymore ^^ I'll look into that later
// 2x
std::string ValToStr(const std::vector<uint8_t> &Bytes,
                     const TargetTypeT TargetType) {

  switch (TargetType) {
  case TargetTypeT::uInt8:
    return std::to_string(readAs<uint8_t>(Bytes));
  case TargetTypeT::uInt16:
    return std::to_string(readAs<uint16_t>(Bytes));
  case TargetTypeT::uInt32:
    return std::to_string(readAs<uint32_t>(Bytes));
  case TargetTypeT::uInt64:
    return std::to_string(readAs<uint64_t>(Bytes));
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
    return std::string(reinterpret_cast<const char *>(Bytes.data()),
                       Bytes.size());
  default:
    Log::Error("Why is TargetType undefined in HitValToStr?? (TargetType: " +
               std::to_string((int)TargetType));
    return {};
  }
}

std::string TargetTypetoStr(const TargetTypeT TargetType) {
  switch (TargetType) {
  case TargetTypeT::uInt8:
    return "uint8";
  case TargetTypeT::uInt16:
    return "uint16";
  case TargetTypeT::uInt32:
    return "uint32";
  case TargetTypeT::uInt64:
    return "uint64";
  case TargetTypeT::Int8:
    return "int8";
  case TargetTypeT::Int16:
    return "int16";
  case TargetTypeT::Int32:
    return "int32";
  case TargetTypeT::Int64:
    return "int64";
  case TargetTypeT::Float:
    return "float";
  case TargetTypeT::Double:
    return "double";
  case TargetTypeT::String:
    return "string";
  case TargetTypeT::Invalid:
  default:
    return "invalid";
  }
}

std::string RelativeStatusToStr(const RelativeStatus Status) {
  switch (Status) {
  case RelativeStatus::INCREASED:
    return "Increased";
  case RelativeStatus::DECREASED:
    return "Decreased";
  case RelativeStatus::UNCHANGED:
    return "Unchanged";
  case RelativeStatus::CHANGED:
    return "Changed";
  case RelativeStatus::UNSET:
    return "Unset...";
  }
  return "";
}

void ContextDisplay::AlignButtons() {
  button_h = ImGui::GetFrameHeight();
  button_w = 150.0f;
  float current_h = ImGui::GetContentRegionAvail().y;

  if (current_h > button_h) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h);
  }

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       (ImGui::GetContentRegionAvail().x - button_w) / 2);
}

bool ContextDisplay::DrawRefreshContextButton() {
  float button_w = 150.0f;

  if (ImGui::Button("Refresh Context Entry", {button_w, 0})) {
    return true;
  }

  return false;
}

bool ContextDisplay::DrawRefreshAllButton() {
  float current_h = ImGui::GetContentRegionAvail().y;
  if (current_h > button_h) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + current_h - button_h);
  }
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ImGui::GetContentRegionAvail().x - button_w);

  if (ImGui::Button("Refresh All Entries", {button_w, 0})) {
    return true;
  }
  return false;
}

float ContextDisplay::DrawRefreshInterval(const float RefreshDuration) {
  float DisplaySeconds = RefreshDuration < 0.3 ? 0 : RefreshDuration;
  int32_t returnval = -2;

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ImGui::GetContentRegionAvail().x - slider_w -
                       checkbox_w - 25);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                       ImGui::GetContentRegionAvail().y - 50);

  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Will regularly refresh entry each given duration.\n");
  }
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ImGui::GetContentRegionAvail().x - slider_w -
                       checkbox_w);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                       ImGui::GetContentRegionAvail().y - 50);
  if (ImGui::Checkbox("##Regular Refresh", &IsRefresh)) {
    if (!IsRefresh) {
      returnval = -1;
    }
    if (IsRefresh)
      returnval = 0;
  }
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       ImGui::GetContentRegionAvail().x - slider_w);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                       ImGui::GetContentRegionAvail().y - 50);

  ImGui::SetNextItemWidth(slider_w);

  if (!IsRefresh)
    ImGui::BeginDisabled();
  if (ImGui::SliderFloat("##interval", &DisplaySeconds, 0.3f, 3.0f, "%.1f",
                         ImGuiSliderFlags_AlwaysClamp)) {
    return DisplaySeconds;
  }

  if (!IsRefresh)
    ImGui::EndDisabled();

  return returnval;
}
