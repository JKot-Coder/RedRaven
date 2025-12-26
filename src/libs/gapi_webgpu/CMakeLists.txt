project (gapi_webgpu)

set(SRC
    BufferImpl.cpp
    BufferImpl.hpp
    CommandListImpl.cpp
    CommandListImpl.hpp
    CommandQueueImpl.cpp
    CommandQueueImpl.hpp
    Device.cpp
    Device.hpp
    DeviceImpl.cpp
    DeviceImpl.hpp
    SwapChainImpl.cpp
    SwapChainImpl.hpp
    TextureImpl.cpp
    TextureImpl.hpp
    TextureViewImpl.cpp
    TextureViewImpl.hpp
    Utils.cpp
    Utils.hpp
    WebGPUImpl.cpp
)
source_group( "" FILES ${SRC} )

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} PRIVATE
    RR::BuildSettings
    common
    gapi
    webgpu
)