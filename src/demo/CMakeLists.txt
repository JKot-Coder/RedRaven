project(demo)

set(definitions)

set(DEMO_BUILD_DEFINITIONS)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(DEMO_BUILD_DEFINITIONS
            DEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(DEMO_BUILD_DEFINITIONS
            RELEASE)
endif()

set(DEMO_LINK_LIBRARIES
        common
        platform
        render
        STB
        ghc_filesystem)

if (WIN32)
    set(DEMO_LINK_LIBRARIES ${DEMO_LINK_LIBRARIES} gapi_dx12)
endif()
        
set(DEMO_DEFINITIONS
        ${DEMO_PLATFORM_DEFINITIONS}
        ${DEMO_BUILD_DEFINITIONS})
       
set(DEMO_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src/libs
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/filesystem
        ${PROJECT_SOURCE_DIR}/input
        ${PROJECT_SOURCE_DIR}/render
        ${SDL2_INCLUDE_DIR})

set(DEMO_SRC_COMMON
        ##inputting/Input.hpp
        main.cpp
        Application.cpp
        Application.hpp)

set(DEMO_SRC_MODULE_FILE_SYSTEM
        filesystem/FileSystem.cpp
        filesystem/FileSystem.hpp
        filesystem/FileStream.cpp
        filesystem/FileStream.hpp)

set(DEMO_SRC
        ${DEMO_SRC_COMMON}
        ${DEMO_SRC_PLATFORM}
        ${DEMO_SRC_MODULE_FILE_SYSTEM}
        )

add_executable(${PROJECT_NAME} WIN32 ${DEMO_SRC})
target_link_libraries(${PROJECT_NAME} ${DEMO_LINK_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${DEMO_DEFINITIONS})
target_include_directories(${PROJECT_NAME} PRIVATE ${DEMO_INCLUDE_DIRS})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)