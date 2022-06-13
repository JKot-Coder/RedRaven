#include "Toolkit.hpp"

#include "glfw/GlfwToolkit.hpp"

namespace RR
{
    namespace Platform
    {
        Toolkit::~Toolkit() { }

        void Toolkit::Init()
        {
            auto toolkit = new GlfwToolkit();
            toolkit->Init();

            impl_.reset(toolkit);
        }
    }
}