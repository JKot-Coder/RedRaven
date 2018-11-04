#include <memory>

#include "Application.hpp"
#include "windowing/Windowing.hpp"

int main(int argc, char** argv) {
    std::shared_ptr<Windowing::IListener> app(static_cast<Windowing::IListener*>(new Application()));

    Windowing::Windowing::Subscribe(app);
    dynamic_cast<Application*>(app.get())->Start();
    Windowing::Windowing::UnSubscribe(app);

    app.reset();
    app = nullptr;

    return 0;
}

