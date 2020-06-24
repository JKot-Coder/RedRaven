#pragma once

#include <chrono>

namespace OpenDemo
{
    namespace Common
    {
        class Time
        {
        public:
            inline Time() { Update(); }

            inline void Init() { Update(); }

            inline float GetDeltaTime() const { return _dt; };

            void Update();

            inline static const std::unique_ptr<Time>& Instance()
            {
                return _instance;
            }

        private:
            float _dt = 0;
            std::chrono::time_point<std::chrono::high_resolution_clock> _lastUpdate;
            static std::unique_ptr<Time> _instance;
        };
    }
}