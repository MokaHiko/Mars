#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#pragma once

#include <string>
#include <unordered_map>

#include "Core/Memory.h"

namespace mrs {
	class Mesh;
	class Texture;
	class Material;
	class Sprite;
	class Font;

	class Model;

	// Responsible for managing runtime resources
	class ResourceManager
	{
	public:
		static ResourceManager& Get() 
		{
			static ResourceManager* rm = new ResourceManager();
			return *rm;
		}

		// Loads a file as raw binary data
		static std::vector<char> LoadBinary(const std::string& path);
	private:
		ResourceManager();
		~ResourceManager();

	public:
		// TODO: Replace with hashed properties
		std::unordered_map<std::string, Ref<Mesh>> _meshes;
		std::unordered_map<std::string, Ref<Texture>> _textures;
		std::unordered_map<std::string, Ref<Material>> _materials;
		std::unordered_map<std::string, Ref<Model>> _models;

		std::unordered_map<std::string, Ref<Font>> _fonts;
		std::unordered_map<std::string, Ref<Sprite>> _sprites;
	};
}

#endif