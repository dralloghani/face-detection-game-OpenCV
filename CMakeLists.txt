cmake_minimum_required(VERSION 3.9)
project(face-detection-game-OpenCV)

set(CMAKE_CXX_STANDARD 11)

#set(SOURCE_FILES main.cpp)

add_executable(face-detection-game-OpenCV main.cpp)

link_directories(..\\..\\mingw-w64/x86_64-7.2.0-posix-seh-rt_v5-rev1/mingw64/lib)

target_link_libraries(face-detection-game-OpenCV mingw32 SDL2main SDL2 SDL2_image SDL2_mixer SDL2_ttf libopencv_world340 freeglut)