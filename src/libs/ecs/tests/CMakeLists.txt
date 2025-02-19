project(ecs_tests)

set(SRC
    "EcsTestsMain.cpp")
source_group( "" FILES ${SRC} )

set(TESTS_SRC
    "BasicTests.cpp"
)
source_group( "Tests" FILES ${TESTS_SRC} )

set(SRC
    ${SRC}
    ${TESTS_SRC})

set(LIBRARIES
    ecs
    common
    Catch2::Catch2)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tests")