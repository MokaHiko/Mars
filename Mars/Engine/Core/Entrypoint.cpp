#include "Core/Application.h"

int main(int, char**) {
	mrs::Application* app = mrs::CreateApplication();
	app->Run();
	app->Shutdown();
	delete app;
	return 0;
}
