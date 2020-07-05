#pragma once

#include "gapi/Command.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class Command
        {
        public:
            enum class Type
            {
                CLEAR_RENDER_TARGET,
                CLEAR_DEPTH_STENCIL,
                CLEAR_UNORDERED_ACCESS,
            };

            Command() = delete;
            Command(Type type)
                : type_(type)
            {
            }

            Type GetType() const
            {
                return type_;
            }

        private:
            Type type_;
        };

    }
}