#ifndef LAYER_H
#define LAYER_H

#pragma once

#include "Events/Events.h"

namespace mrs {
    // Layers are disabled by default
    class Layer
    {
    public:
        Layer() {};
        virtual ~Layer() {};

        virtual void OnAttach() {};
        virtual void OnDetatch() {};

        virtual void OnEnable(){};
        virtual void OnDisable(){};

        virtual void OnUpdate(float dt) {};
        virtual void OnImGuiRender() {};

        virtual void OnEvent(Event& event) {};

        void Enable();
        void Disable();

        inline const std::string& GetName() const {return _name;}

        inline const bool IsEnabled() const {return _enabled;}
    protected:
        friend class LayerStack;

        std::string _name;
        bool _enabled = false;
    };

    // Manages layers and their lifetimes
    class LayerStack
    {
    public:
        LayerStack();
        ~LayerStack();

        void PushLayer(Layer* layer);
        void PopLayer(Layer* layer);

        void EnableLayer(const std::string& layer_name);
        void DisableLayer(const std::string& layer_name);

        std::vector<Layer*>::iterator begin() { return _layers.begin(); } 
        std::vector<Layer*>::iterator end() { return _layers.end(); }
    private:
        std::vector<Layer*> _layers;
        uint32_t _insert_index = 0;
    };
}

#endif