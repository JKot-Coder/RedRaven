project (gapi_dx12)

set(MS_SRC
        d3dx12.h
)
SOURCE_GROUP( "Ms" FILES ${MS_SRC} )

set(IMPL_SRC
        CommandContextImpl.cpp
        CommandContextImpl.hpp
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
SOURCE_GROUP( "Implementations" FILES ${IMPL_SRC} )

set(GAPI_SRC
        pch.hpp
        CommandListCompiler.cpp     
        CommandListCompiler.hpp
        ComSharedPtr.hpp
        Config.hpp
        D3DUtils.cpp
        D3DUtils.hpp
        DescriptorHeap.cpp
        DescriptorHeap.hpp
        DescriptorHeapSet.cpp 
        DescriptorHeapSet.hpp
        Device.cpp
        Device.hpp
        FencedRingBuffer.hpp
        ResourceReleaseContext.hpp
        ResourceCreator.cpp
        ResourceCreator.hpp
        TypeConversions.cpp
        TypeConversions.hpp
        )
SOURCE_GROUP( "" FILES ${GAPI_SRC} )

set( GAPI_SRC
        ${GAPI_SRC}
        ${IMPL_SRC}
        ${MS_SRC}
)
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
target_link_libraries(${PROJECT_NAME} common gapi d3d12.lib dxgi.lib dxguid.lib WindowsApp.lib)
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)