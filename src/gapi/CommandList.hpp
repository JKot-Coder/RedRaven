#pragma once

#include "gapi/Command.hpp"
#include "gapi/Frame.hpp"
#include "gapi/LinearAllocator.hpp"
#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class CommandListAllocator final : public LinearAllocator
        {
        private:
            static inline constexpr size_t BASE_ALLOCATOR_SIZE = 1 << 12;

        public:
            CommandListAllocator()
                : LinearAllocator(BASE_ALLOCATOR_SIZE) {};
            ~CommandListAllocator() = default;

            CommandListAllocator(const CommandListAllocator&) = delete;
            CommandListAllocator& operator=(const CommandListAllocator&) = delete;

            using iterator = std::vector<Command*>::iterator;
            using const_iterator = std::vector<Command*>::const_iterator;

            template <typename Type, typename... Args>
            void emplace_back(Args&&... params)
            {
                static_assert(std::is_base_of<Command, Type>::value);
                static_assert(std::is_trivially_move_constructible<Type>::value);

                void* commandStorage = Allocate(sizeof(Type));
                new (commandStorage) Type();
                commands_.push_back(Create<Type>(std::forward<Args>(params)...));
            }

            Command& front() { return *commands_.front(); }
            const Command& front() const { return *commands_.front(); }
            Command& back() { return *commands_.back(); }
            const Command& back() const { return *commands_.back(); }

            iterator begin() { return commands_.begin(); }
            iterator end() { return commands_.end(); }
            const_iterator begin() const { return commands_.begin(); }
            const_iterator end() const { return commands_.end(); }
            const_iterator cbegin() const { return commands_.cbegin(); }
            const_iterator cend() const { return commands_.cend(); }

        private:
            std::vector<Command*> commands_;
        };

        class CommandList final : public Resource
        {
        public:
            CommandList() = delete;
            CommandList(const CommandList&) = delete;
            CommandList& operator=(const CommandList&) = delete;
            CommandList(const U8String& name)
                : Resource(Resource::Type::CommandList, name)
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