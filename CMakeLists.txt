cmake_minimum_required(VERSION 3.0.0)
project(MarsEngine)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(Vendor)
add_subdirectory(Mars)

# Client Application
add_subdirectory(Rover)