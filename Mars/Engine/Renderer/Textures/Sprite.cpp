#include "Sprite.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <Core/Log.h>

mrs::Sprite::Sprite() 
{
	
}

mrs::Sprite::~Sprite() 
{
}

Ref<mrs::Sprite> mrs::Sprite::Create(Ref<Texture> texture, const std::string& yaml_meta_data, const std::string& alias)
{
	// Calculate sprites
	std::ifstream stream(yaml_meta_data);
	std::stringstream str_stream;
	str_stream << stream.rdbuf();

	YAML::Node data = YAML::Load(str_stream.str());
	auto frames = data["frames"];

	if (!frames)
	{
		MRS_INFO("Failed to create sprite");
		return nullptr;
	}

	std::string name = alias.empty() ? texture->_name : alias;
	auto& sprite = ResourceManager::Get()._sprites[name] = std::make_shared<Sprite>();
	sprite->_atlas = texture;
	for(auto frame : frames)
	{
		Rectangle rect = {};
		rect.x = frame["frame"]["x"].as<float>();
		rect.y = frame["frame"]["y"].as<float>();
		rect.width = frame["frame"]["w"].as<float>();
		rect.height = frame["frame"]["h"].as<float>();

		sprite->_multi_sprites.push_back(rect);
	}

	auto& sprite_rect = sprite->Rect();
	if(sprite->_multi_sprites.size() > 0)
	{
		sprite_rect = sprite->_multi_sprites[0];
		sprite->SetMode(Mode::Multiple);
	}
	else
	{
		sprite_rect.x = 0;
		sprite_rect.y = 0;
		sprite_rect.width= texture->_width;
		sprite_rect.height = texture->_height;

		sprite->SetMode(Mode::Single);
	}

	return sprite;
}

Ref<mrs::Sprite> mrs::Sprite::Create(Ref<Texture> texture, const std::string& alias)
{
	std::string name = alias.empty() ? texture->_name : alias;
	ResourceManager::Get()._sprites[name] = std::make_shared<Sprite>();
	ResourceManager::Get()._sprites[name]->SetMode(Mode::Single);
	ResourceManager::Get()._sprites[name]->_atlas = texture;

	auto& sprite_rect = ResourceManager::Get()._sprites[name]->Rect();
	sprite_rect.x = 0;
	sprite_rect.y = 0;
	sprite_rect.width= texture->_width;
	sprite_rect.height = texture->_height;

	return ResourceManager::Get()._sprites[name];
}

Ref<mrs::Sprite> mrs::Sprite::Get(const std::string& alias)
{
	auto it = ResourceManager::Get()._sprites.find(alias);

	if (it != ResourceManager::Get()._sprites.end()) 
	{
		return it->second;
	}
	return nullptr;
}

mrs::Rectangle& mrs::Sprite::Rect(int index)
{
	if(_mode == Sprite::Mode::Multiple)
	{
		return _multi_sprites[index];
	}

	return _current_rect;
}

const mrs::Sprite::Mode mrs::Sprite::SpriteMode() const
{
	return _mode;
}

void mrs::Sprite::SetMode(Sprite::Mode mode) 
{
	_mode = mode;

	// If mode change re calculate or clear _sprites
}

const int mrs::Sprite::SpriteIndex() const
{
	return _sprite_index;
}

int& mrs::Sprite::SpriteIndex()
{
	return _sprite_index;
}

const int mrs::Sprite::SpriteCount() const
{
	return static_cast<int>(_multi_sprites.size());
}
