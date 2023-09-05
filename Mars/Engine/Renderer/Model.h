#ifndef MODEL_H
#define MODEL_H

#pragma once

#include "Core/Memory.h"

namespace mrs {
    class Mesh;

    // A model is a container for multiple meshes
    class Model
    {
    public:
        Model() {};
        ~Model() {};

		static Ref<Model> LoadFromAsset(const std::string& path, const std::string& alias);
		static Ref<Model> Create(const std::string& alias);
		static Ref<Model> Get(const std::string& alias);

        std::vector<Ref<Mesh>> _meshes;
    };
}

#endif