#pragma once

#include "common/EnumClassOperators.hpp"

#include <unordered_map>

namespace OpenDemo
{
    namespace Common
    {
        template <typename EventEnumClass>
        class EventProvider
        {
        public:
            using CallBackFunction = std::function<void(int)>;

            void RegisterCallback(EventEnumClass eventType, CallBackFunction function);
            void UnRegisterCallback(EventEnumClass eventType, CallBackFunction function);

        protected:
            void FireEvent(EventEnumClass eventType);

        private:
            std::unordered_map<EventEnumClass, std::vector<CallBackFunction>, EnumClassHash> eventsCallbacks_;
        };

        template <typename EventEnumClass>
        inline void EventProvider<EventEnumClass>::RegisterCallback(EventEnumClass eventType, CallBackFunction function)
        {
        }

        template <typename EventEnumClass>
        inline void EventProvider<EventEnumClass>::UnRegisterCallback(EventEnumClass eventType, CallBackFunction function)
        {
        }

        template <typename EventEnumClass>
        inline void EventProvider<EventEnumClass>::FireEvent(EventEnumClass eventType)
        {
            auto& callbacksVector = eventsCallbacks_.find(eventType);

            if (callbacksVector == eventsCallbacks_.end())
            {
                eventsCallbacks_.emplace(eventType, std::vector<CallBackFunction>());
            }

            callbacksVector.
        }
    }
}