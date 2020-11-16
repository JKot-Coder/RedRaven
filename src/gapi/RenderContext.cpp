#include "RenderContext.hpp"

#include "gapi/CommandContext.hpp"
#include "gapi/DeviceInterface.hpp"
#include "gapi/ResourceViews.hpp"
#include "gapi/Result.hpp"
#include "gapi/Submission.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "common/threading/Event.hpp"

namespace OpenDemo
{
    namespace Render
    {
        RenderContext::RenderContext()
            : submission_(new Submission())
        {
        }

        RenderContext::~RenderContext() { }

        Result RenderContext::Init(const Render::PresentOptions& presentOptions)
        {
            ASSERT(!inited_)

            submission_->Start();

            auto result = initDevice();
            if (!result)
            {
                Log::Print::Error("Render device init failed.\n");
                return result;
            }

            result = resetDevice(presentOptions);
            if (!result)
            {
                Log::Print::Error("Render device reset failed.\n");
                return result;
            }

            for (int i = 0; i < presentEvents_.size(); i++)
                presentEvents_[i] = std::make_unique<Threading::Event>(false, true);

            inited_ = true;
            return Result::Ok;
        }

        void RenderContext::Terminate()
        {
            ASSERT(inited_)

            submission_->Terminate();
            inited_ = false;
        }

        void RenderContext::Submit(const CommandContext::SharedPtr& commandContext)
        {
            ASSERT(inited_)

            submission_->Submit(commandContext);
        }

        void RenderContext::Present()
        {
            ASSERT(inited_)

            auto& presentEvent = presentEvents_[presentIndex_];
            // This will limit count of main thread frames ahead.
            presentEvent->Wait();

            submission_->ExecuteAsync([&presentEvent](Render::Device& device) {
                const auto result = device.Present();

                presentEvent->Notify();

                return result;
            });

            if (++presentIndex_ == presentEvents_.size())
                presentIndex_ = 0;
        }

        Render::Result RenderContext::ResetDevice(const PresentOptions& presentOptions)
        {
            ASSERT(inited_)

            return resetDevice(presentOptions);
        }

        CommandContext::SharedPtr RenderContext::CreateRenderCommandContext(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = CommandContext::Create(name);
            if (!submission_->getMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        Texture::SharedPtr RenderContext::CreateTexture(const TextureDescription& desc, Texture::BindFlags bindFlags, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = Texture::Create(desc, bindFlags, name);
            if (!submission_->getMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        RenderTargetView::SharedPtr RenderContext::CreateRenderTargetView(const Texture::SharedPtr& texture, const ResourceViewDescription& desc, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = RenderTargetView::Create(texture, desc, name);
            if (!submission_->getMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        SwapChain::SharedPtr RenderContext::CreateSwapchain(const SwapChainDescription& description, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = SwapChain::Create(description, name);
            if (!submission_->getMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        Render::Result RenderContext::initDevice()
        {
            return submission_->ExecuteAwait([](Render::Device& device) {
                return device.Init();
            });
        }

        Render::Result RenderContext::resetDevice(const PresentOptions& presentOptions)
        {
            return submission_->ExecuteAwait([&presentOptions](Render::Device& device) {
                return device.Reset(presentOptions);
            });
        }
    }
}