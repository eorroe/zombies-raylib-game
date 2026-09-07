cmake_minimum_required(VERSION 3.16)
project(ZombieShooter)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/raylib_src")
set(RAYLIB_INCLUDE_DIR "${RAYLIB_DIR}/src")
set(RAYLIB_LIBRARY "${RAYLIB_DIR}/build/raylib/libraylib.a")

include_directories(${RAYLIB_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${RAYLIB_LIBRARY} m pthread dl GL X11)
