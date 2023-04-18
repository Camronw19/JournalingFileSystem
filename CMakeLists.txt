cmake_minimum_required(VERSION 3.0.0)
project(demo VERSION 0.1.0)

set(CMAKE_TOOLCHAIN_FILE "/home/cam/Documents/vcpkg/scripts/buildsystems/vcpkg.cmake")

add_executable(demo main.cpp myApp.h myApp.cpp)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

target_link_libraries(demo PRIVATE glad::glad glfw imgui::imgui)
