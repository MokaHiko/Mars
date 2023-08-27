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
		// Map between file_path & mesh handle
		std::unordered_map<std::string, Ref<Mesh>> _meshes;

		std::unordered_map<std::string, Ref<Texture>> _textures;

		std::unordered_map<std::string, Ref<Material>> _materials;
	};
}

#endif