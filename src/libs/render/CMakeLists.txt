project (render)

set(SRC
    DeviceContext.cpp
    DeviceContext.hpp
    CommandEncoder.cpp
    CommandEncoder.hpp
    ResourcePointers.hpp
    SwapChain.hpp
    SwapChain.cpp
    RenderTarget.hpp
    Effect.hpp
    Effect.cpp
    EffectManager.cpp
    EffectManager.hpp
    Submission.cpp
    Submission.hpp
)
source_group( "" FILES ${SRC} )

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} PRIVATE RR::BuildSettings common gapi gapi_webgpu effect_library)

if (WIN32)
target_link_libraries(${PROJECT_NAME} PRIVATE gapi_dx12)
endif()