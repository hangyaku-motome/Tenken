cmake_minimum_required(VERSION 3.15)
project(Tenken CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)



set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui SYSTEM PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)



if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()



add_executable(Tenken
    source/main.cpp
    source/ui/display.cpp
    source/ui/LogW.cpp
    source/ui/TargetPopUp.cpp
    source/ui/HitsW.cpp
    source/ui/SearchW.cpp
    source/ui/FavouriteW.cpp
    source/ui/ContextDisplay.cpp
    source/core/Scanner.cpp
    source/core/HitList.cpp
    source/core/FavouriteList.cpp
    source/core/scan_ops.cpp
    source/core/utils.cpp
)
target_include_directories(Tenken PRIVATE
    source
    source/platform
    source/ui
    source/core
)



if(WIN32)
    target_sources(Tenken PRIVATE source/platform/WindowsImpl.cpp)
    target_link_libraries(Tenken PRIVATE imgui)
    target_link_options(Tenken PRIVATE -static)

else()
    target_sources(Tenken PRIVATE source/platform/LinuxImpl.cpp)
    target_link_libraries(Tenken PRIVATE imgui)
endif()



target_compile_options(Tenken PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow
    -Wconversion -Wsign-conversion -Wformat=2
    -Wnull-dereference -Wold-style-cast
    -Woverloaded-virtual -Wnon-virtual-dtor
    -Wno-deprecated-enum-enum-conversion
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(Tenken PRIVATE -g -O0 -fno-omit-frame-pointer)
else()
    target_compile_options(Tenken PRIVATE -O2)
endif()



option(ENABLE_TSAN "Enable thread sanitizer" OFF)
if(ENABLE_TSAN)
    target_compile_options(Tenken PRIVATE -fsanitize=thread)
    target_link_options(Tenken PRIVATE -fsanitize=thread)
endif()



option(ENABLE_SAN "Enable address undefined sanitizer" OFF)
if(ENABLE_SAN)
    target_compile_options(Tenken PRIVATE -fsanitize=address,undefined)
    target_link_options(Tenken PRIVATE -fsanitize=address,undefined)
endif()