#include "IPanel.h"

mrs::IPanel::IPanel(EditorLayer *editor_layer, const std::string &name) 
	:_editor_layer(editor_layer), _name(name) {}

mrs::IPanel::~IPanel() {}
