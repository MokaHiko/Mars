#ifndef SCENESERIALIZER_H
#define SCENESERIALIZER_H

#pragma once

#include "Scene.h"
#include <yaml-cpp/yaml.h>

namespace mrs
{
    // Serializes scene 
    class SceneSerializer
    {
    public:
        SceneSerializer(Scene* scene);
        ~SceneSerializer();

        // Serializes scene into a human readable text format
        void SerializeText(const std::string& scene_name, const std::string& out_path);

        // Deserializes text format scene
        bool DeserializeText(const std::string& scene_path);

        // Serializes scene in binary format
        void SerializeBinary(const std::string& out_path);

    private:
        // Serializes single entityinto yaml emitter
        void SerializeEntity(YAML::Emitter& out, Entity entity);
    private:
        Scene* _scene = nullptr;
    };
}

#endif