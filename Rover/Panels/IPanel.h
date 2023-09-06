#ifndef IPANEL_H
#define IPANEL_H

#pragma once

namespace mrs {
    class EditorLayer;
    class IPanel
    {
    public:
        IPanel(EditorLayer* editor_layer, const std::string& name);
        virtual ~IPanel();

        virtual void Draw() = 0;
    protected:
        EditorLayer* _editor_layer = nullptr;
    private:
        std::string _name = "";
    };
}

#endif