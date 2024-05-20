subdir('third_party/d3d12_memory_allocator')

gapi_dx12_src = files(
    'ComSharedPtr.hpp',
    'Config.hpp',
    'DescriptorHeap.cpp',
    'DescriptorHeap.hpp',
    'DescriptorManager.cpp',
    'DescriptorManager.hpp',
    'Device.cpp',
    'Device.hpp',
    'DeviceContext.cpp',
    'DeviceContext.hpp',
    'InitialDataUploder.cpp',
    'InitialDataUploder.hpp',
    'pch.hpp',
    'ResourceCreator.cpp',
    'ResourceCreator.hpp',
    'ResourceReleaseContext.cpp',
    'ResourceReleaseContext.hpp',
)

gapi_dx12_src += files(
    'CommandListImpl.cpp',
    'CommandListImpl.hpp',
    'CommandQueueImpl.cpp',
    'CommandQueueImpl.hpp',
    'DeviceImpl.cpp',
    'DeviceImpl.hpp',
    'FenceImpl.cpp',
    'FenceImpl.hpp',
    'FramebufferImpl.cpp',
    'FramebufferImpl.hpp',
    'PipelineStateImpl.cpp',
    'PipelineStateImpl.hpp',
    'ResourceImpl.cpp',
    'ResourceImpl.hpp',
    'ResourceViewsImpl.cpp',
    'ResourceViewsImpl.hpp',
    'SwapChainImpl.cpp',
    'SwapChainImpl.hpp',
)

gapi_dx12_src += files(
    'third_party/ms/d3dx12.h',
)

# gapi_dx12_src += files(
#     'third_party/d3d12_memory_allocator/D3D12MemAlloc.h',
#     'third_party/d3d12_memory_allocator/D3D12MemAlloc.cpp',
# )

gapi_dx12_src += files(
    'D3DUtils/D3DUtils.cpp',
    'D3DUtils/D3DUtils.hpp',
    'D3DUtils/DXGIFormatsUtils.cpp',
    'D3DUtils/DXGIFormatsUtils.hpp',
)

cpp = meson.get_compiler('cpp')
d3d12_dep = cpp.find_library('d3d12', required: true)
dxgi_dep = cpp.find_library('dxgi', required: true)
dxguid_dep = cpp.find_library('dxguid', required: true)

gapi_dx12_lib = static_library(
    'gapi_dx12',
    gapi_dx12_src,
    install: true,
    include_directories: include_directories('..'),
    dependencies: [common_dep, gapi_dep, d3d12_dep, d3d12_memory_allocator_dep, dxgi_dep, dxguid_dep],
    cpp_pch: ['pch\pch.hpp', 'pch\pch.cpp'],
)

gapi_dx12_dep = declare_dependency(
    include_directories: include_directories('..'),
    dependencies: [common_dep, gapi_dep, d3d12_dep, d3d12_memory_allocator_dep, dxgi_dep, dxguid_dep],
    link_with: gapi_dx12_lib,
)

#target_link_libraries(${PROJECT_NAME} common gapi d3d12.lib dxgi.lib dxguid.lib WindowsApp.lib)
