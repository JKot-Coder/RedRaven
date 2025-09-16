project(rfxc)

set(RFXC_SRC
    "main.cpp")

source_group( "" FILES ${RFXC_SRC} )

set(RFXC_SRC
    ${RFXC_SRC})

set(RFXC_LINK_LIBRARIES
    RR::BuildSettings
    rfx
    cxxopts)

#================================== Target ===================================#

add_executable(${PROJECT_NAME} ${RFXC_SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${RFXC_LINK_LIBRARIES})
#target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "rfx")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)