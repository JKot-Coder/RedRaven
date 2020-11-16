project (gapi)

set(GAPI_SRC
        CommandContext.hpp
        CommandContextInterface.hpp
        CommandList.hpp
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
        RenderContext.cpp
        RenderContext.hpp
        RenderQueue.hpp
        Resource.cpp
        Resource.hpp
        ResourceViews.cpp
        ResourceViews.hpp
        Result.cpp
        Result.hpp
        Submission.cpp
        Submission.hpp
        SwapChain.cpp
        SwapChain.hpp
        Texture.cpp
        Texture.hpp )
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
target_link_libraries(${PROJECT_NAME} common)