project (ecs)

add_subdirectory(tests)
add_subdirectory(benchmark)

set( SRC
    EntityId.hpp
    Index.hpp
    World.hpp
    World.cpp
    Archetype.hpp
    Archetype.cpp
    ComponentStorage.hpp
    EntityStorage.hpp
    Query.hpp
    Entity.hpp
    Entity.cpp
    EntityBuilder.hpp
    Event.hpp
    System.hpp
    SystemBuilder.hpp
    View.hpp
    Ecs.hpp
    Hash.hpp
    ForwardDeclarations.hpp
    TypeTraits.hpp
    FunctionTraits.hpp
    ComponentTraits.hpp
    IterationHelpers.hpp)
    source_group( "" FILES ${SRC} )

set (SRC
    ${SRC}
)

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/..)
target_link_libraries(${PROJECT_NAME} common flecs::flecs absl::flat_hash_map)

if(MSVC)
    target_compile_optDions(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)