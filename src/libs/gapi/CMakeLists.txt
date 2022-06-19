project (gapi)

set(SRC
    Buffer.cpp
    Buffer.hpp
    Framebuffer.hpp
    CommandList.cpp
    CommandList.hpp
    CommandList.inl
    CommandQueue.hpp
    Device.hpp
    Fence.hpp
    FencedPool.hpp
    ForwardDeclarations.hpp
    Frame.hpp
    GpuResource.cpp
    GpuResource.hpp
    GpuResourceViews.cpp
    GpuResourceViews.hpp
    Limits.hpp
    MemoryAllocation.hpp
    Object.hpp
    Resource.hpp
    SwapChain.cpp
    SwapChain.hpp
    Texture.cpp
    Texture.hpp)
source_group( "" FILES ${SRC} )
        
add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} common)