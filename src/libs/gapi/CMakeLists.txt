project (gapi)

set(GAPI_SRC
        CommandList.hpp
        CommandQueue.hpp
        Device.hpp
        Fence.hpp
        FencedPool.hpp
        ForwardDeclarations.hpp
        Frame.hpp
        Limits.hpp
        LinearAllocator.cpp
        LinearAllocator.hpp
        LinearAllocator.inl
        Object.hpp
        Resource.hpp
        GpuResource.cpp
        GpuResource.hpp
        GpuResourceViews.cpp
        GpuResourceViews.hpp
        Result.cpp
        Result.hpp
        SwapChain.cpp
        SwapChain.hpp
        Texture.cpp
        Texture.hpp
        Buffer.hpp
        Buffer.cpp)
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} common)