#ifndef EVENTS_H
#define EVENTS_H

#pragma once

#include <SDL2/SDL.h>
#include <atomic>

namespace mrs 
{
    // Wrapper for SDL Events
    class Event
    {
    public:
        Event(SDL_Event& e) : _event(e) {}

        void Handle() {_handled = true;}
        bool IsHandled() {return _handled;}

        bool _handled = false;
        SDL_Event& _event;
    };
}

#endif