#include "testES.inl"
#include "flecs.h"

namespace RR
{
    static flecs::query<const Position, const Velocity> q;

    void uupdate()
    {
        if (!q.changed())
            return;

        q.iter(
            [](flecs::iter& it) {
                if (it.changed())
                    Log::Format::Warning("changed\n");
                // Do the thing
            });
    }

    void registwer(flecs::world& ecs)
    {
        q = ecs.query_builder<const Position, const Velocity>().build();
        /*
        ecs.system<Position, Velocity>().each(
            [](Position& p,  Velocity& v)
            {
                Log::Format::Warning("qweqweqwe\n");
         //       p.x += v.x;
          //      p.y += v.y;
           //     v.x += 0.1;
           //    v.y += 0.1;

            });*/

        ecs.system<Position>().each(
            [](Position& p) {
                Log::Format::Warning("kj\n");
                //       p.x += v.x;
                //      p.y += v.y;
                //     v.x += 0.1;
                //    v.y += 0.1;
            });

        ecs.observer<Position, Velocity>("OnSetPosition")
            .event(flecs::OnSet)
            .each(
                [](const Position& p, const Velocity& v) {
                    Log::Format::Warning("   set \n");
                });

        ,

            auto e = ecs.entity().set(
                [](Position& p, Velocity& v) {
                    p = {10, 20};
                    v = {1, 2};
                });
        e.set([](Position& p) { p = {12, 20}; });
    }
}