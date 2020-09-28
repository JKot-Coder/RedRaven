#pragma once

#include "gapi/Command.hpp"
#include "gapi/Frame.hpp"
#include "gapi/LinearAllocator.hpp"
#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class CommandList final : public Object
        {
        public:
            CommandList() = delete;
            CommandList(const CommandList&) = delete;
            CommandList& operator=(const CommandList&) = delete;
            CommandList(const U8String& name)
                : Object(Resource::Type::CommandList, name)
            {
            }

            virtual ~CommandList() = default;

            inline uint64_t GetTargetSubmitFrame() { return targetSubmitFrame_; }
            inline void SetTargetSubmitFrame(uint64_t targetSubmitFrame) { targetSubmitFrame_ = targetSubmitFrame; }

            inline CommandListAllocator* GetAllocator() { return &allocator_; }

        private:
            uint64_t targetSubmitFrame_ = UNDEFINED_FRAME_INDEX;

            CommandListAllocator allocator_;
        };

    }
}