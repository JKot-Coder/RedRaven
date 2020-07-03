#pragma once

namespace OpenDemo
{
    namespace Render
    {
        class Command final
        {
        public:
            enum class Type
            {
                CLEAR_RENDER_TARGET,
                CLEAR_DEPTH_STENCIL,
                CLEAR_UNORDERED_ACCESS,
            };

            Type GetType() const { return type_; }

        private:
            Type type_;
            uint8_t c[40];
        };

    }
}