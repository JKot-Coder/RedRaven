project (gapi_diligent)

set(SRC
    Device.cpp
    Device.hpp
    DeviceImpl.cpp
    DeviceImpl.hpp
    empty.cpp)
source_group( "" FILES ${SRC} )

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME}
    common
    gapi
    Diligent-Common
    Diligent-BuildSettings
    Diligent-GraphicsEngineD3D12-shared
)