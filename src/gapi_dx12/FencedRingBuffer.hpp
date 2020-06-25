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

                    FencedFrameRingBuffer()
                    {
#ifdef ENABLE_FENCE_SYNC_CHECK
                        _fence.reset(new FenceImpl());
#endif
                    }

                    GAPIStatus Init(ID3D12Device* device, NewObjectFunc newFunc)
                    {
                        ASSERT(device && newFunc);

                        GAPIStatus result = GAPIStatus::OK;

                        for (int index = 0; index < GPU_FRAMES_BUFFERED; index++)
                        {
                            auto& object = newFunc();
                            _ringBuffer[index].object = object;
                            if (!object)
                            {
                                Log::Print::Error("Fail to create object in FencedFrameRingBuffer");
                                return GAPIStatus::FAIL;
                            }
#ifdef ENABLE_FENCE_SYNC_CHECK
                            _ringBuffer[index].frameStamp = 0;
#endif
                        }

#ifdef ENABLE_FENCE_SYNC_CHECK
                        if (GAPIStatusU::Failure(result = _fence->Init(device, 1)))
                            return result;
#endif
                        return result;
                    }

                    ObjectType CurrentObject()
                    {
                        ASSERT(_ringBuffer[_frameIndex].object)
#ifdef ENABLE_FENCE_SYNC_CHECK
                        ASSERT(_ringBuffer[_frameIndex].frameStamp <= _fence->GetGpuValue());
#endif
                        return _ringBuffer[_frameIndex].object;
                    }

                    ObjectType GetNextObject()
                    {
#ifdef ENABLE_FENCE_SYNC_CHECK
                        _ringBuffer[_frameIndex].frameStamp = _fence->GetCpuValue();
#endif
                        _frameIndex = (++_frameIndex % GPU_FRAMES_BUFFERED);

#ifdef ENABLE_FENCE_SYNC_CHECK
                        auto fenceGpuValue = _fence->GetGpuValue();
                        ASSERT(_ringBuffer[_frameIndex].frameStamp < fenceGpuValue);
#endif
                        return CurrentObject();
                    }

                    GAPIStatus MoveToNextFrame(ID3D12CommandQueue* commandQueue)
                    {
#ifdef ENABLE_FENCE_SYNC_CHECK
                        return _fence->Signal(commandQueue, _fence->GetCpuValue() + 1);
#else
                        return GAPIStatus::OK;
#endif
                    }

                private:
                    struct Data
                    {
                        ObjectType object = nullptr;
#ifdef ENABLE_FENCE_SYNC_CHECK
                        uint64_t frameStamp = 0;
#endif
                    };

                    std::array<Data, GPU_FRAMES_BUFFERED> _ringBuffer;
                    uint32_t _frameIndex = 0;
#ifdef ENABLE_FENCE_SYNC_CHECK
                    std::unique_ptr<FenceImpl> _fence;
#endif
                };
            }
        }
    }
}