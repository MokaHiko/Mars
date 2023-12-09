#include "EditorManager.h"

void mrs::EditorManager::OnCreate()
{
	MRS_TRACE("Initializing editor manager!");
	InitEditorResources();
}

void mrs::EditorManager::OnStart() {
	_camera_controller = dynamic_cast<EditorCameraController*>(_camera.GetComponent<Script>().script);
}

void mrs::EditorManager::InitEditorResources() {
	// Scene
	Scene* scene = Application::Instance().GetScene();

	// Instantiate Camera
	auto window = Application::Instance().GetWindow();
	_camera = Instantiate("Editor Camera");

	_camera.AddComponent<Camera>(CameraType::Perspective, window->GetWidth(), window->GetHeight());
	_camera.AddScript<EditorCameraController>();
	_camera.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 50.0f);
}
