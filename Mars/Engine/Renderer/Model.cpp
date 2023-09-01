#include "Model.h"

#include "boop.h"

#include "Vulkan/VulkanMesh.h"

Ref<mrs::Model> mrs::Model::LoadFromAsset(const std::string& path, const std::string& alias)
{
	if(Model::Get(path) != nullptr)
	{
		return Model::Get(path);
	}

	Ref<Model> model = CreateRef<Model>();

	if(!alias.empty())
	{
		ResourceManager::Get()._models[alias] = model;
	}
	else
	{
		ResourceManager::Get()._models[alias] = model;
	}

	// Load asset file
    boop::AssetFile file; 
    boop::load(path.c_str(), file);

    return Ref<Model>();
}

Ref<mrs::Model> mrs::Model::Create(const std::string& alias)
{
    return Ref<Model>();
}

Ref<mrs::Model> mrs::Model::Get(const std::string& alias)
{
    return Ref<Model>();
}
