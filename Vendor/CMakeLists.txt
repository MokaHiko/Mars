cmake_minimum_required(VERSION 3.0.0)
project(Vendor)

add_subdirectory(SDL)
add_subdirectory(vkboostrap)
add_subdirectory(glm)
add_subdirectory(astro)
add_subdirectory(boop)
add_subdirectory(entt)

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} 
					INTERFACE boop
					INTERFACE astro
					INTERFACE SDL2::SDL2
					INTERFACE vk-bootstrap::vk-bootstrap
					INTERFACE glm::glm
					INTERFACE EnTT
					)

# Header only libraries 
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE VulkanMemoryAllocator/include)

# Imgui Library
find_package(Vulkan REQUIRED)
add_library(imgui STATIC)
target_include_directories(imgui PUBLIC imgui)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL2::SDL2)
file(GLOB IMGUI_SOURCES "imgui/*.cpp")
file(GLOB IMGUI_HEADERS "imgui/*.h")
target_sources(imgui PRIVATE 
${IMGUI_SOURCES}
${IMGUI_HEADERS}
    )