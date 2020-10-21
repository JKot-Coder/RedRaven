#pragma once

#include "gapi/DeviceInterface.hpp"

#include "common/threading/BufferedChannel.hpp"
#include "common/threading/Thread.hpp"

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

            using CallbackFunction = std::function<void(Render::Device& device)>;

        public:
            struct Work final
            {
            public:
                struct Terminate
                {
                };

                struct Callback
                {
                    CallbackFunction function;
                };
            };

            using WorkVariant = std::variant<Work::Terminate, Work::Callback>;

        public:
            Submission();
            ~Submission();

            void Start();

            Render::Result InitDevice();
            void ExecuteOnSubmission(const CallbackFunction& function, bool waitForExcecution);
            void ResetDevice(const PresentOptions& presentOptions);
            void Terminate();

        private:
            template <typename T>
            void PutWork(const T& work);

        private:
            void threadFunc();

        private:
            std::unique_ptr<Render::Device> device_;
            //   std::unique_ptr<AccessGuard<Render::Device>> device_;
            Common::Threading::Thread submissionThread_;
            Common::Threading::BufferedChannel<WorkVariant, WorkBufferSize> inputWorkChannel_;
        };

    }
}