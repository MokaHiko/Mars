#include "Entity.h"

#include "ECS/Components/Components.h"

mrs::Entity::Entity(entt::entity id, Scene* scene)
	:_id(id), _scene(scene)
{
}

std::ostream& mrs::operator<<(std::ostream& stream, Entity& e)
{
	Tag& tag = e.GetComponent<mrs::Tag>();
	stream << e.Id() << ":" << tag.tag;

	return stream;
}
