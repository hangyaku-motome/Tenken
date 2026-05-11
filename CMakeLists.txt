cmake_minimum_required(VERSION 3.15)
project(Tenken CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
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

target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

add_executable(main 
    source/main.cpp
    source/display.cpp
    source/LogW.cpp
    source/TargetPopUp.cpp
    source/HitsW.cpp
    source/SearchW.cpp
    source/Scanner.cpp
    source/FavouriteW.cpp
)

target_include_directories(main PRIVATE source)

if(WIN32)
    target_sources(main PRIVATE source/platform/platform_windows.cpp)
elseif(UNIX)
    target_sources(main PRIVATE source/platform/LinuxImpl.cpp)
endif()

target_link_libraries(main PRIVATE imgui)

set_target_properties(main PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

target_compile_options(main PRIVATE -fsanitize=address,undefined -g)
target_link_options(main PRIVATE -fsanitize=address,undefined)