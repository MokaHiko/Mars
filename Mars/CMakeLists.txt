cmake_minimum_required(VERSION 3.0.0)
project(Mars VERSION 0.1.0)

include(CTest)
enable_testing()

add_library(${PROJECT_NAME}
			Mars.h 
			Engine/Core/Entrypoint.cpp 
			Engine/Core/Application.h
			Engine/Core/Application.cpp
			Engine/Core/Window.h
			Engine/Core/Window.cpp
			Engine/Util/Utils.h
			Engine/Util/Utils.cpp
			Engine/Renderer/Renderer.h
			Engine/Renderer/Renderer.cpp
			Engine/Renderer/Vulkan/VulkanMesh.h
			Engine/Renderer/Vulkan/VulkanMesh.cpp
			Engine/Renderer/Vulkan/VulkanStructures.h
			Engine/Renderer/Vulkan/VulkanUtils.h
			Engine/Renderer/Vulkan/VulkanUtils.cpp
			Engine/Renderer/Vulkan/VulkanInitializers.h
			Engine/Renderer/Vulkan/VulkanInitializers.cpp
			)

target_include_directories(${PROJECT_NAME} PUBLIC
			Engine
			$ENV{VULKAN_SDK}/include
			${CMAKE_SOURCE_DIR}/Vendor/vkboostrap/src
			${CMAKE_SOURCE_DIR}/Vendor/VulkanMemoryAllocator/include
			)
target_link_directories(${PROJECT_NAME} PUBLIC
			$ENV{VULKAN_SDK}/Lib)
link_libraries(${PROJECT_NAME} PUBLIC
			Vendor
			vma
			vulkan-1)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
