project(rfxc)

set(RFXC_SRC
    "main.cpp")

source_group( "" FILES ${RFXC_SRC} )

set(RFXC_SRC
    ${RFXC_SRC})

set(RFXC_LINK_LIBRARIES
    rfx
    cxxopts)

#================================== Target ===================================#

add_executable(${PROJECT_NAME} ${RFXC_SRC})
target_link_libraries(${PROJECT_NAME} ${RFXC_LINK_LIBRARIES})
#target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "rfx")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)
