#include "Core/Application.h"

int main(int, char**) {
	mrs::Application* app = mrs::CreateApplication();
	delete app;
	return 0;
}
