#ifndef LAYER_H
#define LAYER_H

#pragma once

#include "Events/Events.h"

namespace mrs {
    class Layer
    {
    public:
        Layer() {};
        virtual ~Layer() {};

        virtual void OnAttach() {};
        virtual void OnDetatch() {};

        virtual void OnUpdate(float dt) {};
        virtual void OnImGuiRender() {};

        virtual void OnEvent(Event& event) {};

        inline const std::string& GetName() const {return _name;}

    private:
        std::string _name;
        bool _enabled = true;
    };

    // Manages layers and their lifetimes
    class LayerStack
    {
    public:
        LayerStack();
        ~LayerStack();

        void PushLayer(Layer* layer);
        void PopLayer(Layer* layer);

        std::vector<Layer*>::iterator begin() { return _layers.begin(); } 
        std::vector<Layer*>::iterator end() { return _layers.end(); }
    private:
        std::vector<Layer*> _layers;
        uint32_t _insert_index = 0;
    };
}

#endif