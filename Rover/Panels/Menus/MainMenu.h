#ifndef MAINMENU_H
#define MAINMENU_H

#pragma once

#include "../IPanel.h"

namespace mrs {
    class Scene;
    class MainMenu : public IPanel
    {
    public:
        MainMenu(EditorLayer* editor_layer, const std::string& name, Scene* scene);
        virtual ~MainMenu();

        virtual void Draw() override;
    private:
        Scene* _scene = nullptr;
    };
}


#endif