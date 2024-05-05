gapi_src = files(
    'Buffer.cpp',
    'Buffer.hpp',
    'CommandList.cpp',
    'CommandList.hpp',
    'CommandQueue.hpp',
    'Device.hpp',
    'Fence.hpp',
    'FencedPool.hpp',
    'ForwardDeclarations.hpp',
    'Frame.hpp',
    'Framebuffer.hpp',
    'GpuResource.cpp',
    'GpuResource.hpp',
    'GpuResourceViews.cpp',
    'GpuResourceViews.hpp',
    'Limits.hpp',
    'Object.hpp',
    'PipelineState.hpp',
    'Resource.hpp',
    'SwapChain.cpp',
    'SwapChain.hpp',
    'Texture.cpp',
    'Texture.hpp',
)

gapi_inc = [
    include_directories('..'),
]

gapi_lib = static_library(
    'gapi',
    gapi_src,
    install: true,
    include_directories: gapi_inc,
    dependencies: [common_dep],
    cpp_pch: ['pch\pch.hpp', 'pch\pch.cpp'],
)

gapi_dep = declare_dependency(
    include_directories: gapi_inc,
    dependencies: [common_dep],
    link_with: gapi_lib,
)