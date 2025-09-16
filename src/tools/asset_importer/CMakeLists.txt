project(asset_importer)

set(SRC
    "main.cpp"
    "Meta.hpp"
    "Meta.cpp"
    "Processor.hpp"
    "AssetImporter.hpp"
    "AssetImporter.cpp")
source_group("" FILES ${SRC})


set(SRC_PROCESSORS
    "processors/Effect.hpp"
    "processors/Effect.cpp"
    "processors/Bundle.hpp"
    "processors/Bundle.cpp")
source_group("Processors" FILES ${SRC_PROCESSORS})

set(SRC
    ${SRC}
    ${SRC_PROCESSORS})

set(LIBRARIES
    RR::BuildSettings
    common
    cxxopts
    yaml-cpp
    stduuid
    rfx)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/..")

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tools")