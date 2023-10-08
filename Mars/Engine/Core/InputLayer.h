#ifndef INPUTLAYER_H
#define INPUTLAYER_H

#pragma once

#include "Layer.h"

namespace mrs 
{
    class InputLayer : public Layer
    {
    public:
        InputLayer(){};
        ~InputLayer(){};

        virtual void OnAttach() override;
        virtual void OnEvent(Event& event) override;

        virtual void OnEnable() override;

        virtual void OnImGuiRender() override;
    private:

    };
}

#endif