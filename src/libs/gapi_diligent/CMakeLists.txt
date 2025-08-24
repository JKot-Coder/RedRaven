project (gapi_diligent)

set(SRC
    Utils.cpp
    Utils.hpp
    CommandListImpl.cpp
    CommandListImpl.hpp
    Device.cpp
    Device.hpp
    DeviceImpl.cpp
    DeviceImpl.hpp
    FramebufferImpl.cpp
    FramebufferImpl.hpp
    SwapChainImpl.cpp
    SwapChainImpl.hpp
    GpuResourceImpl.cpp
    GpuResourceImpl.hpp
    GpuResourceViewImpl.cpp
    GpuResourceViewImpl.hpp
    PipelineStateImpl.cpp
    PipelineStateImpl.hpp
    CommandQueueImpl.cpp
    CommandQueueImpl.hpp
    ShaderImpl.cpp
    ShaderImpl.hpp
)
source_group( "" FILES ${SRC} )

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME}
    common
    gapi
    Diligent-Common
    Diligent-GraphicsEngineD3D12-shared
)