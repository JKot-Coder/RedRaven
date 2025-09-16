project (render)

set(SRC
    DeviceContext.cpp
    DeviceContext.hpp
    CommandContext.cpp
    CommandContext.hpp
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
target_link_libraries(${PROJECT_NAME} RR::BuildSettings common gapi gapi_diligent effect_library)