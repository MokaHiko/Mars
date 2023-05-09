cmake_minimum_required(VERSION 3.0.0)
project(Vendor)

add_subdirectory(SDL)
add_subdirectory(vkboostrap)
add_subdirectory(glm)
add_subdirectory(boop)

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} 
					INTERFACE boop
					INTERFACE SDL2::SDL2
					INTERFACE vk-bootstrap::vk-bootstrap
					INTERFACE glm::glm
					)

# Header only libraries 
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE VulkanMemoryAllocator/include)