#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#pragma once

#include <string>
#include <unordered_map>
#include <memory>

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

	private:
		ResourceManager();
		~ResourceManager();

	private:
		friend class Renderer;

		// Map between file_path & mesh handle
		friend class Mesh;
		std::unordered_map<std::string, std::shared_ptr<Mesh>> _meshes;

		friend class Texture;
		std::unordered_map<std::string, std::shared_ptr<Texture>> _textures;

		friend class Material;
		std::unordered_map<std::string, std::shared_ptr<Material>> _materials;
	};
}

#endif