#include "Time.hpp"



namespace Common {

    std::unique_ptr<Time> Time::instance = std::unique_ptr<Time>(new Time());

    void Time::Update() {
        float static invSeconds = 1.0f / std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::seconds(1)).count();

        auto const &now = std::chrono::high_resolution_clock::now();
        dt = (now - lastUpdate).count() * invSeconds;
        lastUpdate = now;
    }


}