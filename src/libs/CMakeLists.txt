add_subdirectory(gapi)
if (WIN32)
add_subdirectory(gapi_dx12)
endif()
add_subdirectory(render)
add_subdirectory(common)
add_subdirectory(stl)
add_subdirectory(platform)
add_subdirectory(inputting)
add_subdirectory(parse_tools)
add_subdirectory(ecs)
add_subdirectory(ecs_module)