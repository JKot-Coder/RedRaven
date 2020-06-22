#include "Time.hpp"

namespace OpenDemo
{
    namespace Common
    {

        std::unique_ptr<Time> Time::_instance = std::unique_ptr<Time>(new Time());

        void Time::Update()
        {
            float static invSeconds = 1.0f / std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::seconds(1)).count();

            auto const& now = std::chrono::high_resolution_clock::now();
            _dt = (now - _lastUpdate).count() * invSeconds;
            _lastUpdate = now;
        }
    }
}