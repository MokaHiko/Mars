cmake_minimum_required(VERSION 3.0.0)
project(Vendor)

add_subdirectory(SDL)
add_subdirectory(vkboostrap)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} 
					INTERFACE SDL2::SDL2
					INTERFACE vk-bootstrap::vk-bootstrap)
