#include "EditorManager.h"

void mrs::EditorManager::OnCreate()
{
	MRS_TRACE("Initializing editor manager!");
	InitEditorResources();
}

void mrs::EditorManager::OnStart() {
	_camera_controller =
		dynamic_cast<CameraController*>(_camera.GetComponent<Script>().script);
}

void mrs::EditorManager::InitEditorResources() {
	// Scene
	Scene* scene = Application::GetInstance().GetScene();

	// Instantiate Camera
	auto window = Application::GetInstance().GetWindow();
	_camera = Instantiate("Editor Camera");

	_camera.AddComponent<Camera>(CameraType::Perspective, window->GetWidth(),
		window->GetHeight());
	_camera.AddComponent<Script>().Bind<CameraController>();
	_camera.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 50.0f);
}
