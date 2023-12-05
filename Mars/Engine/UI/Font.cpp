#include "Font.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <Core/Log.h>

#include "Core/ResourceManager.h"

Ref<mrs::Font> mrs::Font::LoadFromYaml(Ref<Texture> atlas, const std::string& yaml_meta_data, const std::string& alias)
{
	// Calculate sprites
	std::ifstream stream(yaml_meta_data);
	std::stringstream str_stream;
	str_stream << stream.rdbuf();

	YAML::Node data = YAML::Load(str_stream.str());
	auto letters = data["bitmapfont"]["letter"];

	if (!letters)
	{
		MRS_INFO("Failed to create sprite");
		return nullptr;
	}

	auto& font = ResourceManager::Get()._fonts[alias] = std::make_shared<Font>();
	font->_atlas = atlas;
	for(auto letter : letters)
	{
		Rectangle rect = {};
		rect.x = letter["x"].as<float>();
		rect.y = letter["y"].as<float>();
		rect.width = letter["w"].as<float>();
		rect.height = letter["h"].as<float>();

		font->_letters[letter["char"].as<char>()] = rect;
	}

	return font;
}

const mrs::Rectangle& mrs::Font::GetUVS(const char c) const
{
	auto it = _letters.find(static_cast<int>(c));

	if(it != _letters.end())
	{
		return it->second;
	}

	return {};
}
