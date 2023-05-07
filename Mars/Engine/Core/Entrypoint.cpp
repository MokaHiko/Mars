#include "Core/Application.h"

int main(int, char**) {
	mrs::Application* app = mrs::CreateApplication();
	app->Run();
	delete app;
	return 0;
}
