#include "ScriptableEntity.h"
#include "Scene.h"


#include "ECS/Components/Components.h"

namespace mrs 
{
	ScriptableEntity::ScriptableEntity()
	{

	}

	ScriptableEntity::~ScriptableEntity()
	{

	}

	Entity ScriptableEntity::Instantiate(const std::string& name)
	{
		auto e = _game_object._scene->Instantiate(name);

		// Components created at runtime via scripts are not serialized
		e.GetComponent<Serializer>().serialize = false;

		return e;
	}
}