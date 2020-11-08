#pragma once

#include "gapi/DeviceInterface.hpp"

#include "common/threading/BufferedChannel.hpp"
#include "common/threading/Thread.hpp"

#undef NOMINMAX
#pragma warning(push)
#pragma warning(disable : 4267)
#pragma warning(disable : 4996)
#include "dependencies/backward-cpp/backward.hpp"
#pragma warning(pop)
#define NOMINMAX

#include <optional>
#include <variant>

namespace OpenDemo
{
    namespace Render
    {
        class Submission final
        {
            // Queue of 64 work should be enough.
            static constexpr inline size_t WorkBufferSize = 64;

            using CallbackFunction = std::function<Render::Result(Render::Device& device)>;

        public:
            struct Work final
            {
            public:
                Work() = default;
                Work(Work&&) = default;

                struct Terminate
                {
                };

                struct Callback
                {
                    CallbackFunction function;
                };

                using WorkVariant = std::variant<Terminate, Callback>;

            public:
                WorkVariant workVariant;
#ifdef DEBUG
                backward::StackTrace stackTrace;
                U8String label;
#endif
            };

        public:
            Submission();
            ~Submission();

            void Start();
            void Terminate();

            void ExecuteAsync(CallbackFunction&& function);
            Render::Result ExecuteAwait(const CallbackFunction&& function);

            inline std::weak_ptr<Render::MultiThreadDeviceInterface> getMultiThreadDeviceInterface() { return device_; }

        private:
            template <typename T>
            void putWork(T&& work);

        private:
            void threadFunc();

        private:
            std::shared_ptr<Render::Device> device_;
            //   std::unique_ptr<AccessGuard<Render::Device>> device_;
            Common::Threading::Thread submissionThread_;
            Common::Threading::BufferedChannel<Work, WorkBufferSize> inputWorkChannel_;
        };

    }
}