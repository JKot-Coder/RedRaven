project (gapi_dx12)

set(MS_SRC
    third_party/ms/d3dx12.h
)
source_group( "Ms" FILES ${MS_SRC} )

set(D3D12_MEMORY_ALLOCATOR_SRC
    third_party/d3d12_memory_allocator/D3D12MemAlloc.h
    third_party/d3d12_memory_allocator/D3D12MemAlloc.cpp
)
source_group( "D3D12MemoryAllocator" FILES ${D3D12_MEMORY_ALLOCATOR_SRC} )

set(D3D_UTILS_SRC
    D3DUtils/D3DUtils.cpp
    D3DUtils/D3DUtils.hpp
    D3DUtils/DXGIFormatsUtils.cpp
    D3DUtils/DXGIFormatsUtils.hpp
)
source_group( "D3DUtils" FILES ${D3D_UTILS_SRC} )

set(IMPL_SRC
    CommandListImpl.cpp
    CommandListImpl.hpp
    CommandQueueImpl.cpp
    CommandQueueImpl.hpp
    DeviceImpl.cpp
    DeviceImpl.hpp
    FenceImpl.cpp
    FenceImpl.hpp
    FramebufferImpl.cpp
    FramebufferImpl.hpp
    PipelineStateImpl.cpp
    PipelineStateImpl.hpp
    ResourceImpl.cpp
    ResourceImpl.hpp
    ResourceViewsImpl.cpp
    ResourceViewsImpl.hpp
    SwapChainImpl.cpp
    SwapChainImpl.hpp
)
source_group( "Implementations" FILES ${IMPL_SRC} )

set(SRC
    ComSharedPtr.hpp
    Config.hpp
    DescriptorHeap.cpp
    DescriptorHeap.hpp
    DescriptorManager.cpp
    DescriptorManager.hpp
    Device.cpp
    Device.hpp
    DeviceContext.cpp
    DeviceContext.hpp
    InitialDataUploder.cpp
    InitialDataUploder.hpp
    pch.hpp
    ResourceCreator.cpp
    ResourceCreator.hpp
    ResourceReleaseContext.cpp
    ResourceReleaseContext.hpp
)
source_group( "" FILES ${SRC} )

set(SRC
    ${SRC}
    ${IMPL_SRC}
    ${MS_SRC}
    ${D3D_UTILS_SRC}
    ${D3D12_MEMORY_ALLOCATOR_SRC})

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_link_libraries(${PROJECT_NAME} common gapi d3d12.lib dxgi.lib dxguid.lib WindowsApp.lib)
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)
