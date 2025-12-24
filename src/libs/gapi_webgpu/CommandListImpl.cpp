#include "CommandListImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

namespace RR::GAPI::WebGPU
{
    CommandListImpl::~CommandListImpl() { }

    void CommandListImpl::Init(wgpu::Device device)
    {
        wgpu::CommandEncoderDescriptor commandEncoderDescriptor;
        commandEncoderDescriptor.setDefault();

        commandEncoder = device.createCommandEncoder(commandEncoderDescriptor);
    }

    void CommandListImpl::Compile()
    {
        NOT_IMPLEMENTED();
    }
}
