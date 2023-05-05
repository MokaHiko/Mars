#ifndef ENTRYPOINT_H
#define ENTRYPOINT_H

#pragma once

#include "Core/Application.h"

int main(int, char**) {
	mrs::Application* app = mrs::CreateApplication();
	delete app;
	return 0;
}


#endif