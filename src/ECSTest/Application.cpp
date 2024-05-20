#include "Application.hpp"

#include "components.hpp"
#include "flecs.h"

namespace RR
{
    extern void registwer(flecs::world& ecs);
    extern void uupdate();

    namespace Tests
    {
        int Application::Run(int argc, char** argv)
        {
            if (!init())
                return -1;

            flecs::world ecs;
            registwer(ecs);

            ecs.progress();
            uupdate();
            ecs.progress();
            uupdate();

            return 0;
        }

        bool Application::init()
        {

            return true;
        }

        void Application::terminate()
        {
        }
    }
}