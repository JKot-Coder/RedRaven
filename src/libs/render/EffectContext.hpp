#pragma once

namespace RR::Render
{
    class Effect;

    // Staging object for one draw or dispatch call (Phase 9).
    // Stack-allocatable. Snapshots state into the command list at Draw/Dispatch.
    class EffectContext
    {
    public:
        explicit EffectContext(Effect* effect) : effect(effect) { }

        Effect* GetEffect() const { return effect; }

    private:
        Effect* effect = nullptr;
    };
}
