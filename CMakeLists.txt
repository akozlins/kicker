cmake_minimum_required(VERSION 3.20)
project(kicker)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

find_package(imgui REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main OpenGL::GL glfw imgui::imgui)
