project(shader_compiler)

set(SRC
    main.cpp
    ShaderBuilder.cpp
    ShaderBuilder.hpp
    ShaderCompiler.hpp
    ShaderCompiler.cpp
    JsonnetProcessor.cpp
    JsonnetProcessor.hpp
    )
source_group("" FILES ${SRC})

set(SRC
    ${SRC})

set(LIBRARIES
    common
    cxxopts
    nlohmann_json
    slang
    )

 #=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tools")
target_copy_slang_binaries(${PROJECT_NAME})