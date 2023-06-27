#ifndef EDITORMANAGER_H
#define EDITORMANAGER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Renderer/Renderer.h>
#include <Core/Application.h>
#include <Physics/Physics.h>

#include "CameraController.h"
#include "Core/Application.h"

#include "Toolbox/RandomToolBox.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/ParticleRenderPipeline/ParticleComponents.h"

namespace mrs
{
    class EditorManager : public ScriptableEntity
    {
    public:
        EditorManager() {};
        ~EditorManager() {};

        virtual void OnStart() override
        {
            InitEditorResources();
        }

        virtual void OnUpdate(float dt) override {}
        virtual void OnCollisionEnter(Entity other) override {}
    private:
        void InitEditorResources()
        {
        	float z_offset = 0.0f;

			// Scene
			Scene *_scene = Application::GetInstance().GetScene();

			// Camera
			auto window = Application::GetInstance().GetWindow();
			auto camera = _scene->Instantiate("Editor Camera");
			camera.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 20.0f);
			camera.AddComponent<Camera>(CameraType::Perspective, window->GetWidth(), window->GetHeight());
			camera.AddComponent<Script>().Bind<CameraController>();

			// Light source
			Entity _directional_light = _scene->Instantiate("Directional Light");
			_directional_light.AddComponent<RenderableObject>(Mesh::Get("monkey"), Material::Get("default_material"));
			_directional_light.AddComponent<DirectionalLight>();
			_directional_light.GetComponent<Transform>().position = { 150, 900, z_offset };

			// Bawlz
			int s = 1;
            tbx::PRNGenerator<float> rand_engine{-10, 10};
			for (int i = 0; i < s; i++)
			{
				Entity e = _scene->Instantiate("Ball");
				e.GetComponent<Transform>().position = { rand_engine.Next(), rand_engine.Next(), z_offset };
				e.AddComponent<RenderableObject>(Mesh::Get("sphere"), Material::Get("default_material"));

				e.AddComponent<RigidBody2D>();
				e.AddComponent<BoxCollider2D>();

				auto& particles = e.AddComponent<ParticleSystem>(); 
				particles.max_particles = 512;
				particles.life_time = 0.6f;
				particles.emission_rate = 128.0f;
				particles.velocity = { 50.0f, 50.0f };
				particles.repeating = true;
				particles.texture = Material::Get("green_material");
			}

			// Crate
			{
				Entity _test_object = _scene->Instantiate("Container");
				_test_object.AddComponent<RenderableObject>(Mesh::Get("container"), Material::Get("container"));

				// _test_object.AddComponent<RigidBody2D>();
				// _test_object.AddComponent<BoxCollider2D>();

				_test_object.GetComponent<Transform>().position = { 0, 10, z_offset };
				_test_object.GetComponent<Transform>().scale = glm::vec3{ 0.1f };
			}

			// Floor
			{
				Entity stage = _scene->Instantiate("Stage");
				stage.GetComponent<Transform>().position = glm::vec3{ 0.0f, -10.0f, z_offset };
				stage.GetComponent<Transform>().scale = glm::vec3{ 1000.0f, 1.0f, 1.0f };
				stage.AddComponent<RenderableObject>(Mesh::Get("plane"), Material::Get("default_material"));

				stage.AddComponent<RigidBody2D>().type = BodyType::STATIC;
				stage.AddComponent<BoxCollider2D>();
			}     
        };
    };
}

#endif