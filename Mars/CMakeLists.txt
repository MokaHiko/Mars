cmake_minimum_required(VERSION 3.0.0)
project(Mars VERSION 0.1.0)

include(CTest)
enable_testing()

add_library(${PROJECT_NAME}
			Mars.h 
			Engine/Core/Entrypoint.cpp 
			Engine/Core/Application.h
			Engine/Core/Application.cpp
			Engine/Core/Layer.h
			Engine/Core/Layer.cpp
			Engine/Core/Window.h
			Engine/Core/Window.cpp
			Engine/Core/Input.h
			Engine/Core/Input.cpp
			Engine/Core/ResourceManager.h
			Engine/Core/ResourceManager.cpp
			Engine/ECS/Scene.h
			Engine/ECS/Scene.cpp
			Engine/ECS/Entity.h
			Engine/ECS/Entity.cpp
			Engine/ECS/Components/Components.h
			Engine/ECS/Components/Components.cpp
			Engine/Util/Utils.h
			Engine/Util/Utils.cpp
			Engine/Renderer/Renderer.h
			Engine/Renderer/Renderer.cpp
			Engine/Renderer/Camera.h
			Engine/Renderer/Camera.cpp
			Engine/Renderer/Vulkan/VulkanMesh.h
			Engine/Renderer/Vulkan/VulkanMesh.cpp
			Engine/Renderer/Vulkan/VulkanTexture.h
			Engine/Renderer/Vulkan/VulkanTexture.cpp
			Engine/Renderer/Vulkan/VulkanMaterial.h
			Engine/Renderer/Vulkan/VulkanMaterial.cpp
			Engine/Renderer/Vulkan/VulkanStructures.h
			Engine/Renderer/Vulkan/VulkanUtils.h
			Engine/Renderer/Vulkan/VulkanUtils.cpp
			Engine/Renderer/Vulkan/VulkanInitializers.h
			Engine/Renderer/Vulkan/VulkanInitializers.cpp
			Engine/ImGui/MarsImGui.h
			Engine/ImGui/MarsImGui.cpp
			)
target_precompile_headers(${PROJECT_NAME} PUBLIC mrspch.h)
target_include_directories(${PROJECT_NAME} PUBLIC
			Engine
			$ENV{VULKAN_SDK}/include
			)
target_link_directories(${PROJECT_NAME} PUBLIC
			$ENV{VULKAN_SDK}/Lib)
target_link_libraries(${PROJECT_NAME} PUBLIC
			Vendor
			vma
			imgui
			vulkan-1)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
