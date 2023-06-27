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

        virtual void OnAttach();
        virtual void OnEvent(Event& event) override;
    private:

    };
}

#endif