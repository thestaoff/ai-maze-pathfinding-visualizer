cmake_minimum_required(VERSION 3.16)

project(AIMazePathfindingVisualizer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(maze_visualizer main.cpp)
target_link_libraries(maze_visualizer PRIVATE sfml-graphics sfml-window sfml-system)
