project (ecs)

include(${CMAKE_CURRENT_LIST_DIR}/EcsSources.cmake)
source_group( "" FILES ${ECS_SRC} )

add_subdirectory(tests)
add_subdirectory(benchmark)

set (SRC
    ${ECS_SRC}
)

add_library(${PROJECT_NAME} STATIC ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/..)
target_link_libraries(${PROJECT_NAME} common absl::flat_hash_map)

if(MSVC)
    target_compile_optDions(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)