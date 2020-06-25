#pragma once

#include "gapi_dx12/FenceImpl.hpp"

#include <functional>

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {
                template <typename ObjectType>
                class FencedFrameRingBuffer
                {
                public:
                    using NewObjectFunc = std::function<ObjectType()>;

                    FencedFrameRingBuffer() = delete;

                    FencedFrameRingBuffer(std::shared_ptr<FenceImpl>& fence, NewObjectFunc newFunc)
                        : _fence(fence)
                        , _newObjFunc(newFunc)
                    {
                        ASSERT(fence && newFunc);

                        for (int index = 0; index < GPU_FRAMES_BUFFERED; index++)
                        {
                            _ringBuffer[index].object = createObject();
                            _ringBuffer[index].frameStamp = 0;
                        }
                    }

                    ObjectType CurrentObject()
                    {
                        ASSERT(_ringBuffer[_frameIndex].frameStamp <= _fence->GetGpuValue());
                        return _ringBuffer[_frameIndex].object;
                    }

                    ObjectType GetNextObject()
                    {
                        auto frameStamp = _fence->GetGpuValue();
                        ASSERT(_ringBuffer[_frameIndex].frameStamp < frameStamp);
                        _ringBuffer[_frameIndex].frameStamp = _fence->GetCpuValue();

                        _frameIndex = (++_frameIndex % GPU_FRAMES_BUFFERED);
                        return CurrentObject();
                    }

                private:
                    ObjectType createObject()
                    {
                        ObjectType obj = _newObjFunc();
                        if (obj == nullptr)
                            Log::Print::Fatal("Failed to create new object in fenced frame ring buffer");
                        return obj;
                    }

                    struct Data
                    {
                        ObjectType object;
                        uint64_t frameStamp;
                    };

                    NewObjectFunc _newObjFunc = nullptr;
                    std::array<Data, GPU_FRAMES_BUFFERED> _ringBuffer;
                    uint32_t _frameIndex = 0;
                    std::shared_ptr<FenceImpl>& _fence;
                };
            }
        }
    }
}