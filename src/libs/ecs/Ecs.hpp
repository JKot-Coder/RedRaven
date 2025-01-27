#pragma once

#include "World.hpp"
#include "Event.hpp"
#include "EventBuilder.hpp"
#include "SystemBuilder.hpp"

#if ECS_CODEGEN
#define ECS_SYSTEM __attribute__((annotate("@ecs_system")))
#else
#define ECS_SYSTEM
#endif
