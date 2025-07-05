project(ecs_benchmark)

set(SRC
    "EcsBenchmarkMain.cpp"
    "Benchmark.cpp")
source_group( "" FILES ${SRC} )

set(LIBRARIES
    ecs
    common
    flecs::flecs
    Catch2::Catch2
    nanobench::nanobench)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "benchmarks")