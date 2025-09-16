project(ecs_codegen)

set(SRC main.cpp)
set(ECS_CPP_FILES)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

foreach(ECS_CPP ${ECS_CPP_FILES})
    get_filename_component(ABS_ECS_CPP ${ECS_CPP} ABSOLUTE)
    get_filename_component(FILE_NAME ${ECS_CPP} NAME_WE)
    get_filename_component(FILE_DIR ${ABS_ECS_CPP} DIRECTORY)
    set(GEN_FILE "${FILE_DIR}/${FILE_NAME}.ecs.gen.cpp")

    add_custom_command(
        OUTPUT ${GEN_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "Generating RTTR for ${ECS_CPP}"
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/codegen.py ${ABS_ECS_CPP} ${GEN_FILE}
        DEPENDS ${ECS_CPP} ${CMAKE_CURRENT_SOURCE_DIR}/codegen.py
        COMMENT "Running RTTR codegen for ${ECS_CPP}"
        VERBATIM
    )

    list(APPEND GEN_CPP_FILES ${GEN_FILE})
endforeach()

add_executable(${PROJECT_NAME} ${SRC} ${GEN_CPP_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE meta)