cmake_minimum_required(VERSION 3.16)
project(ZombieShooter)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/raylib_src")
set(RAYLIB_INCLUDE_SEARCH_DIR "${RAYLIB_DIR}/src")
set(RAYLIB_LIBRARY_SEARCH_DIR "${RAYLIB_DIR}/build/raylib")

find_path(RAYLIB_INCLUDE_DIR NAMES raylib.h PATHS "${RAYLIB_INCLUDE_SEARCH_DIR}" NO_DEFAULT_PATH)
if(NOT RAYLIB_INCLUDE_DIR)
    message(FATAL_ERROR "raylib.h not found under: ${RAYLIB_INCLUDE_SEARCH_DIR}\nEnsure raylib_src exists and is a valid raylib checkout.")
endif()

find_library(RAYLIB_LIBRARY NAMES raylib libraylib PATHS "${RAYLIB_LIBRARY_SEARCH_DIR}" NO_DEFAULT_PATH)
if(NOT RAYLIB_LIBRARY)
    message(FATAL_ERROR "raylib library not found under: ${RAYLIB_LIBRARY_SEARCH_DIR}\nBuild raylib first (run run.bat or run.sh).")
endif()

include_directories(${RAYLIB_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} ${RAYLIB_LIBRARY} opengl32 gdi32 winmm shell32)
else()
    target_link_libraries(${PROJECT_NAME} ${RAYLIB_LIBRARY} m pthread dl GL X11)
endif()
