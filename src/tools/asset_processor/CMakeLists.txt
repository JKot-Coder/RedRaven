project(asset_processor)

set(SRC
    "main.cpp")
source_group("" FILES ${SRC})

set(SRC 
    ${SRC})

set(LIBRARIES)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)