#pragma once

#include "ecs/ForwardDeclarations.hpp"

struct GLFWwindow;
namespace RR::Ecs
{
    struct Window
    {
        GLFWwindow* window_ = nullptr;
    };

    void InitWindowModule(World& world);
}