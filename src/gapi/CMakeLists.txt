project (gapi)

set(GAPI_SRC
        Frame.hpp
        Limits.hpp
        Object.hpp
        Resource.hpp
        Resource.cpp
        ResourceViews.hpp
        ResourceViews.cpp
        LinearAllocator.hpp
        LinearAllocator.cpp
        LinearAllocator.inl
        Submission.hpp
        Submission.cpp
	    CommandList.hpp
        Fence.hpp
        FencedPool.hpp
        Texture.hpp
        Texture.cpp
        Result.hpp
        Result.cpp
        RenderContext.hpp
        RenderContext.cpp
        DeviceInterface.hpp
        CommandContext.hpp
        CommandContextInterface.hpp
        RenderQueue.hpp )
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
target_link_libraries(${PROJECT_NAME} common)