#ifndef SIGNALTOOLBOX_H
#define SIGNALTOOLBOX_H

#pragma once

#include <mutex>

namespace tbx
{
    template<typename... Args>
    class SignalHandler
    {
    public:
        typedef std::function<void(Args...)> SignalHandlerFn;

        // Copy constructor
        SignalHandler(const SignalHandler& other)
            :_handler_fn(other._handler_fn), _id(other._id) {}

        SignalHandler(SignalHandler&& other)
            :_handler_fn(std::move(other._handler_fn)), _id(other._id) {}

        SignalHandler& operator=(const SignalHandler& other)
        {
            _handler_fn = other._handler_fn;
            _id = other._id;
        }

        SignalHandler& operator=(const SignalHandler&& other)
        {
            std::swap(_handler_fn, _other._handler_fn);
            _id = other._id;

            return *this;
        }

        SignalHandler() = default;
        SignalHandler(const SignalHandlerFn& fn)
            :_handler_fn(fn) {}

        void operator()(Args... params) const
        {
            if(_handler_fn)
            {
                _handler_fn(params...);
            }
        }

        const uint32_t Id() const
        {
            return _id;
        }

        SignalHandlerFn _handler_fn;
    private:
        // Unique id for handler
        uint32_t _id;

        // Atomic id counter
        static std::atomic_uint _handler_id_counter;
    };

    template<typename... Args>
    class Signal
    {
    public:
        Signal() = default;
        Signal(const Signal& signal)
        {
            _signal_handlers = signal._signal_handlers;
        }

        Signal(Signal&& signal)
        {
            _signal_handlers = std::move(signal._signal_handlers);
        }

        Signal& operator=(const Signal& signal)
        {
            _signal_handlers = signal._signal_handlers;

            return *this;
        }

        Signal& operator=(Signal&& src)
        {
            std::swap(_signal_handlers, src._signal_handlers);

            return *this;
        }
    public:
        typedef SignalHandler<Args...> SignalHandlerType;
        typedef std::vector<SignalHandlerType> SignalHandlerCollectionType;

        // Add signal handler to signal
        uint32_t Add(const SignalHandlerType& signal_handler)
        {
            _signal_handlers.push_back(signal_handler);
            return signal_handler.Id();
        }

        // Add signal handler to by function
        uint32_t Add(const typename SignalHandlerType::SignalHandlerFn& handler_fn)
        {
            return Add(SignalHandlerType(handler_fn));
        }

        // Remove handler
        bool Remove(const SignalHandlerType& signal_handler)
        {
            auto it = std::find(_signal_handlers.begin(), _signal_handlers.end(), signal_handler);
            if(it != _signal_handlers.end())
            {
                _signal_handlers.erase(it);
                return true;
            }

            return false;
        }

        // Remove by handler id
        bool Remove(uint32_t handler_id)
        {
            auto it = std::find_if(_signal_handlers.begin(), _signal_handlers.end(), [handler_id](const SignalHandlerType& signal_handler){
                return signal_handler.Id() ==  handler_id;
            });

            if(it != _signal_handlers.end())
            {
                _signal_handlers.erase(it);
                return true;
            }

            return false;
        }

        // Call all signal handlers
        void Call(Args... params) const
        {
            for (auto& signal_handler : _signal_handlers)
            {
                signal_handler(params...);
            }
        }

        // Calls all signal listener handlers
        inline void operator()(Args... params) const
        {
            Call(params...);
        }

        inline typename uint32_t operator+=(const SignalHandlerType& handler)
        {
            return Add(handler);
        }

        inline typename uint32_t operator+=(const typename SignalHandlerType::SignalHandlerFn& callback)
        {
            return Add(callback);
        }

        inline bool operator-=(const SignalHandlerType& handler)
        {
            return Remove(handler);
        }

    private:
        SignalHandlerCollectionType _signal_handlers;
    };
}

#endif