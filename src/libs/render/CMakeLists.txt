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
)
source_group( "" FILES ${SRC} )

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} common gapi gapi_diligent effect_library)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)