cmake_minimum_required(VERSION 3.0.0)
project(Rover VERSION 0.1.0)

add_executable(Rover
	"Rover.cpp"
	"Scripts/EditorManager.h"
	"Scripts/EditorManager.cpp"
	"Scripts/CameraController.h"
	"Scripts/CameraController.cpp"
)

target_link_libraries(${PROJECT_NAME} PUBLIC Mars Vendor imgui vulkan-1)
target_include_directories(${PROJECT_NAME} PUBLIC "../Mars")

set(MRS_BUILD_CONFIGURATION "Debug")
set(MRS_APP_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${MRS_BUILD_CONFIGURATION}")

# Copy resources
add_custom_target(copy_assets ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_SOURCE_DIR}/Assets
	${MRS_APP_OUTPUT_DIR}/Assets
	COMMENT "Copying resources into bin and binary folder."
	COMMAND cmd /c ${MRS_APP_OUTPUT_DIR}/Assets/Shaders/compile_shaders.bat
	COMMENT "Compiling shaders.")

add_dependencies(${PROJECT_NAME} copy_assets)
