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


set(IMPL_SRC
        DeviceImpl.cpp
        DeviceImpl.hpp
        CommandListImpl.cpp
        CommandListImpl.hpp
        CommandQueueImpl.cpp
        CommandQueueImpl.hpp
        FenceImpl.cpp
        FenceImpl.hpp
        ResourceImpl.cpp
        ResourceImpl.hpp
        ResourceViewsImpl.cpp
        ResourceViewsImpl.hpp
        SwapChainImpl.cpp
        SwapChainImpl.hpp
)
source_group( "Implementations" FILES ${IMPL_SRC} )

set(GAPI_SRC
        pch.hpp
        ComSharedPtr.hpp
        Config.hpp
        D3DUtils.cpp
        D3DUtils.hpp
        GpuMemoryHeap.hpp
        GpuMemoryHeap.cpp
        DescriptorHeap.cpp
        DescriptorHeap.hpp
        IntermediateMemoryAllocator.hpp
        IntermediateMemoryAllocator.cpp
        DescriptorHeapSet.cpp 
        DescriptorHeapSet.hpp
        DeviceContext.hpp
        DeviceContext.cpp
        Device.cpp
        Device.hpp
        ResourceReleaseContext.hpp
        #InitialUploadsManager.hpp
        #InitialUploadsManager.cpp
        ResourceCreator.cpp
        ResourceCreator.hpp
        TypeConversions.cpp
        TypeConversions.hpp
        )
source_group( "" FILES ${GAPI_SRC} )

set( GAPI_SRC
        ${GAPI_SRC}
        ${IMPL_SRC}
        ${MS_SRC}
        ${D3D12_MEMORY_ALLOCATOR_SRC}
)
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_link_libraries(${PROJECT_NAME} common gapi d3d12.lib dxgi.lib dxguid.lib WindowsApp.lib)
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)
