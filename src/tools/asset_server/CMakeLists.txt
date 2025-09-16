project(asset_server)

set(SRC
    "AssetServer.hpp"
    "AssetServer.cpp"
    "main.cpp")
source_group("" FILES ${SRC})


set(SRC
    ${SRC})

set(LIBRARIES
    RR::BuildSettings
    common
    cxxopts
    yaml-cpp
    stduuid
    efsw)

#=============================== Target ===============================#

add_library(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/..")

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tools")