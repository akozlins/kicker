#

cmake_minimum_required(VERSION 3.14)
project(kicker)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
include_directories(${GTKMM_INCLUDE_DIRS})

add_executable(main main.cpp)
target_link_libraries(main ${GTKMM_LIBRARIES})
