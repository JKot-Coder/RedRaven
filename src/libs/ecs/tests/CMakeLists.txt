project(ecs_tests)

set(SRC
    "EcsTestsMain.cpp")
source_group( "" FILES ${SRC} )

set(TESTS_SRC
    "EntityAndComponents.cpp"
    "ViewsAndQueries.cpp"
    "Systems.cpp"
    "Tracking.cpp"
    "Traits.cpp"
    "TestHelpers.hpp"
    "Meta.cpp"
)
source_group( "Tests" FILES ${TESTS_SRC} )

include(${CMAKE_CURRENT_LIST_DIR}/../EcsSources.cmake)
source_group( "" FILES ${ECS_SRC} )

set(SRC
    ${SRC}
    ${ECS_SRC}
    ${TESTS_SRC})

set(LIBRARIES
    common
    absl::flat_hash_map
    Catch2::Catch2)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} ${SRC} )
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../..  ${PROJECT_SOURCE_DIR}/.. )
target_compile_definitions(${PROJECT_NAME} PRIVATE ECS_USE_EXCEPTIONS)

target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tests")