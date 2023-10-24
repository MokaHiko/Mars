#ifndef MODEL_H
#define MODEL_H

#pragma once

#include "Core/Memory.h"

namespace mrs {
    class Mesh;
    class Material;

    struct ModelMesh
    {
        Ref<Mesh> mesh;
        int material_index;
    };

    // A model is a container for multiple meshes. Effectively the basic 3d node of a scene
    class Model
    {
    public:
        Model() {};
        ~Model() {};

		static Ref<Model> LoadFromAsset(const std::string& path, bool flip_uvs = false, const std::string& alias = "");
		static Ref<Model> Create(const std::string& alias);
		static Ref<Model> Get(const std::string& alias);

        const std::string& Name() const;
        std::string& Name();

        std::string _name;
        std::vector<ModelMesh> _meshes;
        std::vector<Ref<Material>> _materials;
    };
}

#endif