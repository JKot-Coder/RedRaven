project (gapi)

set(GAPI_SRC
        CommandList.hpp
        CommandQueue.hpp
        DeviceInterface.hpp
        Fence.hpp
        FencedPool.hpp
        ForwardDeclarations.hpp
        Frame.hpp
        Limits.hpp
        LinearAllocator.cpp
        LinearAllocator.hpp
        LinearAllocator.inl
        Object.hpp
        GpuResource.cpp
        GpuResource.hpp
        GpuResourceViews.cpp
        GpuResourceViews.hpp
        Result.cpp
        Result.hpp
        SwapChain.cpp
        SwapChain.hpp
        Texture.cpp
        Texture.hpp        )
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
target_link_libraries(${PROJECT_NAME} common)