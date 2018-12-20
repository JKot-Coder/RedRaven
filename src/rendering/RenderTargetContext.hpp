#pragma once

#include <cstdint>
#include <memory>

namespace Rendering {

    class RenderTarget;

    enum class RenderTargetIndex;

    class RenderTargetContext {
    public:

        inline void SetDepthStencilTarget(const std::shared_ptr<RenderTarget>& renderTarget) {
            deptStencil = renderTarget;
        }

        void SetColorTarget(RenderTargetIndex index, const std::shared_ptr<RenderTarget>& renderTarget){
            colorTargets[index] = renderTarget;
        }

        virtual void Bind() = 0;
    private:
        std::shared_ptr<RenderTarget> deptStencil;
        std::shared_ptr<RenderTarget> colorTargets[RenderTargetIndex::INDEX_MAX];
    };

}