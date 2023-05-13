#include "Layer.h"


mrs::LayerStack::LayerStack()
	:_insert_index(0) {}

mrs::LayerStack::~LayerStack()
{
	for (Layer* layer : _layers) {
		delete layer;
	}
}

void mrs::LayerStack::PushLayer(Layer* layer)
{
	_layers.emplace(_layers.begin() + _insert_index, layer);
	_insert_index++;
}

void mrs::LayerStack::PopLayer(Layer* layer)
{
	auto it = std::find(_layers.begin(), _layers.end(), layer);

	if (it != _layers.end()) {
		_layers.erase(it);
		_insert_index--;
	}
}
