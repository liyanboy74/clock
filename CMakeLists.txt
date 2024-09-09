cmake_minimum_required(VERSION 3.6)

add_subdirectory(color)
add_subdirectory(simlcd)
add_subdirectory(dispcolor)

project(clock)

add_executable(${PROJECT_NAME} main.cpp)

include_directories(${SDL2_INCLUDE_DIRS} simlcd color dispcolor dispcolor/fonts)
target_link_libraries(${PROJECT_NAME} dispcolor simlcd color )

