#include <iostream>

#include <Mars.h>


class SandboxApp : public mrs::Application
{
public:
	SandboxApp() {};
	~SandboxApp() {};
};

mrs::Application* mrs::CreateApplication()
{
	return new SandboxApp();
}

