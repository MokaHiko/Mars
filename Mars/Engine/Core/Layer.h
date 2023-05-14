#ifndef LAYER_H
#define LAYER_H

#pragma once


#include <vector>
#include <cstdint>

namespace mrs {

    class Layer
    {
    public:
        Layer() {};
        virtual ~Layer() {};

        virtual void OnAttach() {};
        virtual void OnDetatch() {};

        virtual void OnUpdate(float dt) {};
    };

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