#include "windowing/Windowing.hpp"
#include "windowing/WindowSettings.hpp"

int main(int argc, char** argv) {
    Windowing::WindowSettings settings;
    Windowing::Windowing::CreateWindow(settings);
}

