project(shader_compiler)

set(SRC
    main.cpp
    ShaderBuilder.cpp
    ShaderBuilder.hpp
    ShaderCompiler.hpp
    ShaderCompiler.cpp
    JsonnetProcessor.cpp
    JsonnetProcessor.hpp
    ReflectionBuilder.hpp
    ReflectionBuilder.cpp
    ReflectionResourceType.hpp
    )
source_group("" FILES ${SRC})

set(SRC
    ${SRC})

set(LIBRARIES
    RR::BuildSettings
    common
    cxxopts
    nlohmann_json
    slang
    absl::flat_hash_map
    )

 #=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tools")
target_copy_slang_binaries(${PROJECT_NAME})