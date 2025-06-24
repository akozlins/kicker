cmake_minimum_required(VERSION 3.20)
project(kicker)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include(FetchContent)
FetchContent_Populate(imgui QUIET
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG 97428e8 # v1.91.9
)
file(GLOB IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/*.cpp
    ${imgui_SOURCE_DIR}/*.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(main main.cpp ${IMGUI_SOURCES})
target_link_libraries(main PRIVATE OpenGL::GL glfw)
target_include_directories(main PRIVATE ${imgui_SOURCE_DIR})
