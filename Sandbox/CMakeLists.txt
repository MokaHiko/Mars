cmake_minimum_required(VERSION 3.0.0)
project(Sandbox VERSION 0.1.0)

add_executable(Sandbox main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC Mars Vendor vulkan-1)

target_include_directories(${PROJECT_NAME} PUBLIC "../Mars")