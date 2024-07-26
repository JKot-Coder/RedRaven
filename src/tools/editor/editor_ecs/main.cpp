#include "flecs.h"

#include "cr.h"

struct TaskBar{};

int init(flecs::world& world)
{
    world.entity().add<TaskBar>();

    world.system<TaskBar>("TaskBar")
    .each([](const TaskBar&) {

    });

    return 0;
}

CR_EXPORT int cr_main(struct cr_plugin *ctx, enum cr_op operation) {
    assert(ctx);
    assert(ctx->userdata);
    switch (operation) {
        case CR_LOAD:   return init(*(flecs::world*) ctx->userdata); // loading back from a reload
        case CR_UNLOAD: return 0; // preparing to a new reload
        case CR_CLOSE:  return 0; // the plugin will close and not reload anymore
        default: return 0; // CR_STEP
    }
}