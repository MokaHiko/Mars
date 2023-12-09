#include "Entity.h"

#include "ECS/Components/Components.h"

mrs::Entity::Entity(entt::entity id, Scene* scene)
	:_id(id), _scene(scene)
{
}

bool mrs::Entity::IsAlive() const
{
	if(this->_id == entt::null)
	{
		return false;
	}

	if(!_scene)
	{
		return false;
	}

	if(!_scene->Registry()->valid(_id))
	{
		return false;
	}

	return true;
}

std::ostream& mrs::operator<<(std::ostream& stream, Entity& e)
{
	Tag& tag = e.GetComponent<mrs::Tag>();
	stream << e.Id() << ":" << tag.tag;
	
	return stream;
}
