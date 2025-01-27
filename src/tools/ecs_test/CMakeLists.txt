project(ecs_test)

set(SRC
    "main.cpp")
source_group("" FILES ${SRC})

set(SRC
    ${SRC})

set(LIBRARIES)

# Collect all ECS source files
file(GLOB_RECURSE ECS_CPP_FILES "${CMAKE_SOURCE_DIR}/*.ecs.inl")

#get_target_property(INCLUDE_DIRS ${PROJECT_NAME} INCLUDE_DIRECTORIES)
#string(REPLACE ";" " " INCLUDE_ARGS "${INCLUDE_DIRS}")
#message(WARNING $INCLUDE_ARGS)

# set(GENERATED_FILES)
# foreach(ECS_FILE ${ECS_CPP_FILES})
#     get_filename_component(ECS_NAME ${ECS_FILE} NAME_WE)
#     set(GENERATED_FILE "${CMAKE_SOURCE_DIR}/${ECS_NAME}.gen.cpp")

#     add_custom_command(
#         OUTPUT ${GENERATED_FILE}
#         COMMAND ecs_codegen ${ECS_FILE} ${GENERATED_FILE} /I${INCLUDE_ARGS}
#         DEPENDS ecs_codegen ${ECS_FILE}
#         COMMENT "Generating ${GENERATED_FILE}"
#     )

#     list(APPEND GENERATED_FILES ${GENERATED_FILE})
# endforeach()

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} ${SRC} ${GENERATED_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tools")