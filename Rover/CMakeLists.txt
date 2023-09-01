cmake_minimum_required(VERSION 3.0.0)
project(Rover VERSION 0.1.0)

add_executable(Rover
	"Rover.cpp"
	"UIHelpers.cpp"
	"Scripts/Editor/EditorManager.h"
	"Scripts/Editor/EditorManager.cpp"
	"Scripts/Editor/CameraController.h"
	"Scripts/Editor/CameraController.cpp"
	"Panels/Components/ComponentPanels.h"
	"Panels/Components/ComponentPanels.cpp"
	"Panels/InspectorPanel.h"
	"Panels/InspectorPanel.cpp"
	"Panels/Menus/MainMenu.h"
	"Panels/Menus/MainMenu.cpp"
	"Panels/Menus/Viewport.h"
	"Panels/Menus/Viewport.cpp"

	"GameLayer/GameLayer.h"
	"GameLayer/GameLayer.cpp"
	"GameLayer/Scripts/Unit.h"
	"GameLayer/Scripts/Unit.cpp"
	"GameLayer/Scripts/Spawner.h"
	"GameLayer/Scripts/Spawner.cpp"
	"GameLayer/Scripts/GameManager.h"
	"GameLayer/Scripts/GameManager.cpp"
)

target_link_libraries(${PROJECT_NAME} PUBLIC Mars Vendor imgui vulkan-1)
target_include_directories(${PROJECT_NAME} PUBLIC "../Mars" "./")

set(MRS_BUILD_CONFIGURATION "Debug")
set(MRS_APP_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${MRS_BUILD_CONFIGURATION}")

if(MRS_BUILD_CONFIGURATION STRLESS_EQUAL "Debug")
	add_compile_definitions(MRS_DEBUG)
endif()

# Copy resources
add_custom_target(copy_assets ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_SOURCE_DIR}/Assets
	${MRS_APP_OUTPUT_DIR}/Assets
	COMMENT "Copying resources into bin and binary folder."
	COMMAND cmd /c ${MRS_APP_OUTPUT_DIR}/Assets/Shaders/compile_shaders.bat
	COMMENT "Compiling shaders.")

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${MRS_APP_OUTPUT_DIR})

add_dependencies(${PROJECT_NAME} copy_assets)
