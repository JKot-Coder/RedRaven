#include <memory>

#include "Application.hpp"
#include "windowing/Windowing.hpp"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    auto app = new OpenDemo::Application;

    app->Start();

    return 0;
}

