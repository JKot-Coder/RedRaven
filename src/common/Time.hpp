#pragma once

#include <memory>
#include <chrono>

namespace Common {

    class Time {
    public:
        inline Time() { Update(); }

        inline void Init() { Update(); }

        inline float GetDeltaTime() const { return dt; };

        void Update();

        inline static const std::unique_ptr<Time>& Instance() {
            return instance;
        }

    private:
        float dt = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdate;
        static std::unique_ptr<Time> instance;
    };

}