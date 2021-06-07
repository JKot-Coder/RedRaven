#pragma once

#include "common/EnumClassOperators.hpp"
#include <map>

namespace RR
{
    namespace Common
    {
        // This class has problems: to unsubscribe need to provide the exact same lamba which we subscribe.
        // So, we should hold all lambdas for subscribe and unsubscribe.
        // This class is candidate to remove

        template <typename EventEnumClass>
        class EventProvider
        {
            static_assert(std::is_enum<EventEnumClass>::value, "Must be a scoped enum!");
            static_assert(!std::is_convertible<EventEnumClass, typename std::underlying_type<EventEnumClass>::type>::value,
                          "Must be a scoped enum!");

        public:
            using CallBackFunction = std::function<void()>;

            void Subscribe(EventEnumClass eventType, CallBackFunction function)
            {
                const size_t functionAdress = *reinterpret_cast<size_t*>(reinterpret_cast<char*>(&function));
                Log::Print::Info("Adresss: %zx \n", functionAdress);

                eventsCallbacks_.insert(std::make_pair(eventType, function));
            }

            void Unsubscribe(EventEnumClass eventType, CallBackFunction function)
            {
                const size_t functionAdress = *reinterpret_cast<size_t*>(reinterpret_cast<char*>(&function));
                const auto& subscribers = eventsCallbacks_.equal_range(eventType);

                Log::Print::Info("Adresss: %zx \n", functionAdress);

                for (auto it = subscribers.first; it != subscribers.second; it++)
                {
                    const size_t subscriberAdress = *reinterpret_cast<long*>(reinterpret_cast<char*>(&(*it)));
                    if (subscriberAdress == functionAdress)
                        eventsCallbacks_.erase(it);
                }
            }

        protected:
            void FireEvent(EventEnumClass eventType)
            {
                const auto& equalRange = eventsCallbacks_.equal_range(eventType);

                for (auto it = equalRange.first; it != equalRange.second; it++)
                {
                    //it.second->();
                }
            }

        private:
            std::multimap<EventEnumClass, CallBackFunction> eventsCallbacks_;
        };
    }
}