#include "NativeScriptComponent.h"
#include "NativeScriptingLayer.h"

void mrs::Script::OnBind(Entity e)
{
    static auto scripting_layer = (NativeScriptingLayer*)Application::Instance().FindLayer("NativeScriptingLayer");
	scripting_layer->OnScriptBound(e, this);
}
