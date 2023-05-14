cmake_minimum_required(VERSION 3.0.0)
project(Rover VERSION 0.1.0)

add_executable(Rover main.cpp ImGui/ImGuiLayer.cpp ImGui/ImGuiLayer.h)

target_link_libraries(${PROJECT_NAME} PUBLIC Mars Vendor imgui vulkan-1)

target_include_directories(${PROJECT_NAME} PUBLIC "../Mars")

# Copy resources
add_custom_target(copy_assets ALL
COMMAND ${CMAKE_COMMAND} -E copy_directory
${PROJECT_SOURCE_DIR}/Assets
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets
COMMAND ${CMAKE_COMMAND} -E copy_directory
${PROJECT_SOURCE_DIR}/Assets
${PROJECT_BINARY_DIR}/Assets
COMMENT "Copying resources into bin and binary folder.")

add_dependencies(${PROJECT_NAME} copy_assets)
