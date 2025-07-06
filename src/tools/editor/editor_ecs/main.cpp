#include "cr.h"
#include "ecs\Ecs.hpp"
#include "ecs_module\Context.hpp"
#include <imgui.h>


#pragma clang optimize off
int init(const RR::EcsModule::Context& ctx)
{
    UNUSED(ctx);
    return 0;
}

CR_EXPORT int cr_main(struct cr_plugin* ctx, enum cr_op operation)
{
    assert(ctx);
    assert(ctx->userdata);
    switch (operation)
    {
        case CR_LOAD: return init(*(RR::EcsModule::Context*)ctx->userdata); // loading back from a reload
        case CR_UNLOAD: return 0; // preparing to a new reload
        case CR_CLOSE: return 0; // the plugin will close and not reload anymore
        default: return 0; // CR_STEP
    }
}
#pragma clang optimize on