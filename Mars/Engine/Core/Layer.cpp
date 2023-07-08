#include "Layer.h"
#include "Log.h"

mrs::LayerStack::LayerStack()
	:_insert_index(0) {}

mrs::LayerStack::~LayerStack()
{
	for (Layer *layer : _layers)
	{
		delete layer;
	}
}

void mrs::LayerStack::PushLayer(Layer *layer)
{
	_layers.emplace(_layers.begin() + _insert_index, layer);
	_insert_index++;
}

void mrs::LayerStack::PopLayer(Layer *layer)
{
	auto it = std::find(_layers.begin(), _layers.end(), layer);

	if (it != _layers.end())
	{
		_layers.erase(it);
		_insert_index--;
	}
}

void mrs::LayerStack::EnableLayer(const std::string &layer_name)
{
	for(Layer* layer: _layers)
	{
		if(layer->GetName() == layer_name)
		{
			layer->Enable();
			return;
		}
	}

	MRS_ERROR("No layer with such name");
}

void mrs::LayerStack::DisableLayer(const std::string & layer_name)
{
	for(Layer* layer: _layers)
	{
		if(layer->GetName() == layer_name)
		{
			layer->Disable();
			return;
		}
	}

	MRS_ERROR("No layer with such name");
}

void mrs::Layer::Enable() 
{
	_enabled = true;
	OnEnable();
}

void mrs::Layer::Disable() 
{
	_enabled = false;
	OnDisable();
}
