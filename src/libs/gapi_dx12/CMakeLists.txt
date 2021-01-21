project (gapi_dx12)

set(MS_SRC
        d3dx12.h
)
source_group( "Ms" FILES ${MS_SRC} )

set(IMPL_SRC
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
        DescriptorHeap.cpp
        DescriptorHeap.hpp
        DescriptorHeapSet.cpp 
        DescriptorHeapSet.hpp
        Device.cpp
        Device.hpp
        ResourceReleaseContext.hpp
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
)
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_link_libraries(${PROJECT_NAME} common gapi d3d12.lib dxgi.lib dxguid.lib WindowsApp.lib)
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)
