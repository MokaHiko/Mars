#include "ScriptableEntity.h"
#include "Scene.h"

#include "ECS/Components/Components.h"
#include "Scripting/ProcessLayer.h"

#include "Core/Log.h"
#include "Math/Math.h"

namespace mrs 
{
	ScriptableEntity::ScriptableEntity()
	{

	}

	ScriptableEntity::~ScriptableEntity()
	{

	}

	Entity ScriptableEntity::Instantiate(const std::string& name, Vector3 position)
	{
		auto e = _game_object._scene->Instantiate(name, position);

		// Components created at runtime via scripts are not serialized
		e.GetComponent<Serializer>().serialize = false;

		return e;
	}

	void ScriptableEntity::StartProcess(Ref<Process> process)
	{
		if(!process)
		{
			return;
		}

		if(process->IsAlive())
		{
			MRS_ERROR("Process already started!");
		}

		static ProcessLayer* process_layer = dynamic_cast<ProcessLayer*>(Application::Instance().FindLayer("ProcessLayer"));

		if(process_layer)
		{
			process_layer->AttachProcess(process);
		}
		else
		{
			MRS_ERROR("Process Layer not found!");
		}
	}
}