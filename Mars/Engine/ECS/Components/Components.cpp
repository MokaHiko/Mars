#include "Components.h"

std::unordered_map <std::string, std::function<mrs::ScriptableEntity* ()>> mrs::Script::script_instantiation_bindings;