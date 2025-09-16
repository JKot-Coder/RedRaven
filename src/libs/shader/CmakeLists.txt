project(shader)

set(SRC
    main.cpp)
source_group( "" FILES ${SRC} )

set(SRC
    ${SRC})

set(SHADER_SRC
    "shader.jsonnet")

set(LIBRARIES)

#=============================== Target ===============================#

add_library(${PROJECT_NAME} STATIC ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/.. )
target_add_shaders(${PROJECT_NAME} SHADERS ${SHADER_SRC})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)