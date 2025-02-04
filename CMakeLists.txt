# Minimum required CMake version
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(RTSP_Client C)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavutil libswscale)

# Add executable
add_executable(RTSPClient.bin RTSPClient.c)

# Include directories for SDL2 and FFmpeg
include_directories(${SDL2_INCLUDE_DIRS} ${FFMPEG_INCLUDE_DIRS})

# Link necessary libraries
target_link_libraries(RTSPClient.bin ${SDL2_LIBRARIES} ${FFMPEG_LIBRARIES})

# Define custom install directory relative to the project root
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

# Install rules
install(TARGETS RTSPClient.bin DESTINATION bin)
install(FILES README.md DESTINATION share)
