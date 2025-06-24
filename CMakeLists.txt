cmake_minimum_required(VERSION 3.14)
project(kicker)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)


file(GLOB IMGUI_SOURCES
    ${PROJECT_SOURCE_DIR}/imgui/*.cpp
    ${PROJECT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${PROJECT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(main main.cpp ${IMGUI_SOURCES})
target_link_libraries(main OpenGL::GL glfw)
