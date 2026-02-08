project (gapi)

set(SRC
    Buffer.cpp
    Buffer.hpp
    BindingLayout.hpp
    BindingGroup.hpp
    CommandList.hpp
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
    PipelineState.hpp
    Resource.hpp
    Shader.hpp
    SwapChain.cpp
    SwapChain.hpp
    Texture.hpp
    VertexLayout.hpp
)

set(COMMANDS
    Commands/Command.hpp
    Commands/RenderPass.hpp
    Commands/Draw.hpp
)

source_group( "" FILES ${SRC} ${COMMANDS} )

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} PRIVATE common)