#pragma once

#include "gapi/Result.hpp"

#include "gapi/CommandContext.hpp"
#include "gapi/Resource.hpp"
#include "gapi/ResourceViews.hpp"
#include "gapi/Texture.hpp"

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


            void Submit(const CommandContext::SharedPtr& commandContext);
            void Present();
            Result ResetDevice(const PresentOptions& presentOptions);

            CommandContext::SharedPtr CreateRenderCommandContext(const U8String& name) const;
            Texture::SharedPtr CreateTexture(const Texture::Description& desc, Resource::BindFlags bindFlags, const U8String& name = "") const;

            RenderTargetView::SharedPtr CreateRenderTargetView(const Texture::SharedPtr& texture, const ResourceView::Description& desc, const U8String& name) const;

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