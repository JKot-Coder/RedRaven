#pragma once

#include "gapi/Result.hpp"

#include "common/Singleton.hpp"

#include <array>

namespace OpenDemo
{
    namespace Common
    {
        namespace Threading
        {
            class Event;
        }
    }

    namespace Render
    {
        class Submission;
        struct PresentOptions;

        // Todo thread safity?
        class RenderContext final : public Singleton<RenderContext>
        {
        public:
            RenderContext();
            ~RenderContext();

            Result Init(const PresentOptions& presentOptions);
            void Terminate();

            void Present();
            Result ResetDevice(const PresentOptions& presentOptions);
            Result CreateRenderCommandContext();

        private:
            Render::Result initDevice();
            Render::Result resetDevice(const PresentOptions& presentOptions);

        private:
            static constexpr int SubmissionThreadAheadFrames = 4;

            bool inited_ = false;

            std::array<std::unique_ptr<Threading::Event>, SubmissionThreadAheadFrames> presentEvents_;
            int presentIndex_ = 0;

            std::unique_ptr<Submission> submission_;
        };
    }
}