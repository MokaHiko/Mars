#ifndef ROVER_H
#define ROVER_H

#pragma once

#include <Mars.h>

namespace mrs {
	class NativeScriptingLayer;
	class IRenderPipelineLayer;
	class IPanel;
	class EditorLayer : public Layer
	{
	public:
		enum class EditorState : uint8_t
		{
			Uknown,
			Playing, 
			Paused,
			Stopped,
		};

        const EditorState State() const {return _state;}
        void SetEditorState(EditorState state) {_state = state;}

		const Entity& SelectedEntity() const {return _selected_entity;}
		void SetSelectedEntity(Entity e) {_selected_entity = e;}

		void FocusEntity(Entity entity);

		virtual void OnAttach() override;
		virtual void OnDetatch() override;

		virtual void OnEvent(Event &event);
		virtual void OnEnable();

		virtual void OnUpdate(float dt) override;
		virtual void OnImGuiRender() override;
	public:
		void Play();
		void Pause();
		void Stop();
    private:
		void LoadEditorResources();
		void LoadEditorScene();	
	private:
		float _mouse_sensitivity = 0.5f;
		bool _constrain_pitch = true;
		float _movement_speed = 50.0f;

		Entity _editor_camera = {};
		Entity _selected_entity = {};

		EditorState _state;

		NativeScriptingLayer* _native_scripting_layer = nullptr;
		IRenderPipelineLayer* _render_pipeline_layer = nullptr;

		std::vector<Ref<IPanel>> _panels;
	};

	class Rover : public Application
	{
	public:
		Rover();
		~Rover();
	};
}

#endif