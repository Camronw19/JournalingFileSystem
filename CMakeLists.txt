cmake_minimum_required(VERSION 3.0.0)
project(demo VERSION 0.1.0)

set(CMAKE_TOOLCHAIN_FILE "/home/cam/Documents/vcpkg/scripts/buildsystems/vcpkg.cmake")

link_directories(./Dependencies/inotify-cpp)

find_library(inotify-cpp libinotify-cpp.so.0.2.0 PATHS ./Dependencies/inotify-cpp)
if (NOT inotify-cpp)
    message(FATAL_ERROR "Library not found - inotify-cpp")
endif()

find_library(glad libglad.a PATHS ./Dependencies/glad)
if (NOT inotify-cpp)
    message(FATAL_ERROR "Library not found - glad")
endif()

add_executable(demo src/main.cpp src/myApp.cpp src/JournalUtils.cpp)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

target_link_libraries(demo PRIVATE ${glad} glfw imgui::imgui ${inotify-cpp})
