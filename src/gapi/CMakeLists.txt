project (gapi)

set(GAPI_SRC
        Frame.hpp
        Limits.hpp
        Object.hpp
        Resource.hpp
        ResourceViews.hpp
        LinearAllocator.hpp
        LinearAllocator.cpp
        LinearAllocator.inl
	    CommandList.hpp
        Fence.hpp
        FencedPool.hpp
        Texture.hpp
        Texture.cpp
        Result.hpp
        Result.cpp
        DeviceInterface.hpp
        RenderContextInterface.hpp
        RenderQueue.hpp )
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
target_link_libraries(${PROJECT_NAME} common)