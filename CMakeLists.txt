cmake_minimum_required(VERSION 3.15)
project(My_MemScanner CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_library(imgui STATIC
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC 
    third_party/imgui
    third_party/imgui/backends
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

add_executable(main 
    source/main.cpp
    source/display.cpp
    source/LogW.cpp
    source/TargetPopUp.cpp
    source/HitsW.cpp
    source/SearchW.cpp
    source/platform/platform_linux.cpp
)

target_include_directories(main PRIVATE source)

if(WIN32)
    target_sources(main PRIVATE source/platform/platform_windows.cpp)
elseif(UNIX)
    target_sources(main PRIVATE source/platform/platform_linux.cpp)
endif()

target_link_libraries(main PRIVATE imgui)

set_target_properties(main PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

