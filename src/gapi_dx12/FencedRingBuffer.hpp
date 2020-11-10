#pragma once

#include "gapi_dx12/FenceImpl.hpp"

#include <functional>

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            template <typename ObjectType>
            class FencedFrameRingBuffer
            {
            public:
                using NewObjectFunc = std::function<Result(int, ObjectType&)>;

                FencedFrameRingBuffer()
                {
                    static_assert(std::is_base_of<ID3D12Object, std::remove_pointer<ObjectType>::type>::value, "Wrong type for FencedFrameRingBuffer");
#ifdef ENABLE_FENCE_SYNC_CHECK
                    _fence.reset(new FenceImpl());
#endif
                }

                ~FencedFrameRingBuffer()
                {
                    for (int index = 0; index < GPU_FRAMES_BUFFERED; index++)
                        if (_ringBuffer[index].object)
                            _ringBuffer[index].object->Release();
                }

                Result Init(ComSharedPtr<ID3D12Device> device, NewObjectFunc newObject, const U8String& name)
                {
                    ASSERT(newObject);

                    for (int index = 0; index < GPU_FRAMES_BUFFERED; index++)
                    {
                        ObjectType object;

                        D3DCallMsg(newObject(index, object), "Fail to create object in FencedFrameRingBuffer");

                        _ringBuffer[index].object = object;
#ifdef ENABLE_FENCE_SYNC_CHECK
                        _ringBuffer[index].frameStamp = 0;
#endif
                    }

#ifdef ENABLE_FENCE_SYNC_CHECK
                    D3DCall(_fence->Init(device, 1, fmt::format("FencedFrameRingBuffer::{}", name)));

#endif
                    return Result::Ok;
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

                Result MoveToNextFrame(ID3D12CommandQueue* commandQueue)
                {
#ifdef ENABLE_FENCE_SYNC_CHECK
                    return _fence->Signal(commandQueue, _fence->GetCpuValue() + 1);
#else
                    return Result::Ok;
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