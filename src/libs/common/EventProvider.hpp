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
            static_assert(std::is_enum<EventEnumClass>::value, "Must be a scoped enum!");
            static_assert(!std::is_convertible<EventEnumClass, typename std::underlying_type<EventEnumClass>::type>::value,
                          "Must be a scoped enum!");

        public:
            using CallBackFunction = std::function<void(int)>;

            void Subscribe(EventEnumClass eventType, CallBackFunction function)
            {
                std::ignore = function;

                auto callbacksVector = eventsCallbacks_.find(eventType);

                if (callbacksVector == eventsCallbacks_.end())
                {
                    eventsCallbacks_.emplace(eventType, std::vector<CallBackFunction>());
                }
            }

            void Unsubscribe(EventEnumClass eventType, CallBackFunction function)
            {
                std::ignore = eventType;
                std::ignore = function;
            }

        protected:
            void FireEvent(EventEnumClass eventType)
            {
                auto& callbacksVector = eventsCallbacks_.find(eventType);

                if (callbacksVector == eventsCallbacks_.end())
                {
                    eventsCallbacks_.emplace(eventType, std::vector<CallBackFunction>());
                }

                callbacksVector.
            }

        private:
            std::unordered_map<EventEnumClass, std::vector<CallBackFunction>, EnumClassHash> eventsCallbacks_;
        };
    }
}