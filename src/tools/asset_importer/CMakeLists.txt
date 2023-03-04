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
    common
    cxxopts
    yaml-cpp
    stduuid
    rfx)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)
