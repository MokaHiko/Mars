cmake_minimum_required(VERSION 3.0.0)
project(Rover VERSION 0.1.0)

add_executable(Rover
	"Rover.h"
	"Rover.cpp"
	"UIHelpers.cpp"
	"Scripts/Spawner.h"
	"Scripts/Spawner.cpp"
	"Scripts/Editor/CameraController.h"
	"Scripts/Editor/CameraController.cpp"
	"Scripts/Editor/EditorManager.h"
	"Scripts/Editor/EditorManager.cpp"
	"Scripts/Editor/EditorCameraController.h"
	"Scripts/Editor/EditorCameraController.cpp"

	"Panels/IPanel.h"
	"Panels/IPanel.cpp"
	"Panels/Viewport.h"
	"Panels/Viewport.cpp"
	"Panels/Components/ComponentPanels.h"
	"Panels/Components/ComponentPanels.cpp"
	"Panels/InspectorPanel.h"
	"Panels/InspectorPanel.cpp"
	"Panels/HierarchyPanel.h"
	"Panels/HierarchyPanel.cpp"
	"Panels/PerformancePanel.h"
	"Panels/PerformancePanel.cpp"
	"Panels/Menus/MainMenu.h"
	"Panels/Menus/MainMenu.cpp"

	"GameLayer/GameLayer.h"
	"GameLayer/GameLayer.cpp"

	"GameLayer/RenderPipelines/CBRenderPipeline.h"
	"GameLayer/RenderPipelines/CBRenderPipeline.cpp"

	"GameLayer/Scripts/Ship.h"
	"GameLayer/Scripts/Ship.cpp"
	"GameLayer/Scripts/ShipMovement.h"
	"GameLayer/Scripts/ShipMovement.cpp"
	"GameLayer/Scripts/ShipCombat.h"
	"GameLayer/Scripts/ShipCombat.cpp"

	"GameLayer/Scripts/Weapon/Weapon.h"
	"GameLayer/Scripts/Weapon/Weapon.cpp"
	"GameLayer/Scripts/Weapon/Projectile/Projectile.h"
	"GameLayer/Scripts/Weapon/Projectile/Projectile.cpp"

	"GameLayer/Scripts/Weapon/ChargeCanon.cpp"
	"GameLayer/Scripts/Weapon/ChargeCanon.h"
	"GameLayer/Scripts/Weapon/Projectile/Bullet.h"
	"GameLayer/Scripts/Weapon/Projectile/Bullet.cpp"

	"GameLayer/Scripts/Weapon/GravGun.h"
	"GameLayer/Scripts/Weapon/GravGun.cpp"

	"GameLayer/Scripts/Effects/Effect.h"
	"GameLayer/Scripts/Effects/Effect.cpp"
	"GameLayer/Scripts/Effects/TracerEffect.h"
	"GameLayer/Scripts/Effects/TracerEffect.cpp"
	"GameLayer/Scripts/Effects/TargetLockEffect.h"
	"GameLayer/Scripts/Effects/TargetLockEffect.cpp"

	"GameLayer/Scripts/Scrap.h"
	"GameLayer/Scripts/Scrap.cpp"
	"GameLayer/Scripts/PlayerShipController.h"
	"GameLayer/Scripts/PlayerShipController.cpp"
	"GameLayer/Scripts/Striker.h"
	"GameLayer/Scripts/Striker.cpp"
	"GameLayer/Scripts/Planet.h"
	"GameLayer/Scripts/Planet.cpp"
	"GameLayer/Scripts/Moon.h"
	"GameLayer/Scripts/Moon.cpp"
	"GameLayer/Scripts/GameCamera.h"
	"GameLayer/Scripts/GameCamera.cpp"
	"GameLayer/Scripts/GameManager.h"
	"GameLayer/Scripts/GameManager.cpp"
)

target_link_libraries(${PROJECT_NAME} PUBLIC Mars Vendor imgui vulkan-1)
target_include_directories(${PROJECT_NAME} PUBLIC "../Mars" "./" "GameLayer/Scripts")

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
