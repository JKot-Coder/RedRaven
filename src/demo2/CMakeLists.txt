project(demo2)

set(SRC
    Application.cpp
    Application.hpp
    main.cpp)
source_group("" FILES ${SRC})

set(LIBRARIES
    common
    ecs
    ecs_window
    render_loom
)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

copy_required_dlls(demo2)
