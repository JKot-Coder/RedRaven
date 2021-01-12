project(OpenDemo)

set(definitions)

set(OPEN_DEMO_BUILD_DEFINITIONS)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(OPEN_DEMO_BUILD_DEFINITIONS
            DEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(OPEN_DEMO_BUILD_DEFINITIONS
            RELEASE)
endif()


set(OPEN_DEMO_LINK_LIBRARIES
        ${SDL2_LIBRARY}
        common
        gapi_dx12
        render
        STB
        ghc_filesystem
        CONAN_PKG::Assimp
        CONAN_PKG::sdl2)
        
set(OPEN_DEMO_DEFINITIONS
        ${OPEN_DEMO_PLATFORM_DEFINITIONS}
        ${OPEN_DEMO_BUILD_DEFINITIONS})
       
set(OPEN_DEMO_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src/libs
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/filesystem
        ${PROJECT_SOURCE_DIR}/input
        ${PROJECT_SOURCE_DIR}/windowing
        ${PROJECT_SOURCE_DIR}/render
        ${SDL2_INCLUDE_DIR})

set(OPEN_DEMO_SRC_COMMON
        ##inputting/Input.hpp
        main.cpp
        Application.cpp
        Application.hpp)

set(OPEN_DEMO_SRC_MODULE_FILE_SYSTEM
        filesystem/FileSystem.cpp
        filesystem/FileSystem.hpp
        filesystem/FileStream.cpp
        filesystem/FileStream.hpp)

set(OPEN_DEMO_SRC_MODULE_INPUTTING
        inputting/Input.hpp
        inputting/Input.cpp)

set(OPEN_DEMO_SRC_MODULE_WINDOWING
        windowing/Window.hpp
		windowing/Window.cpp
		windowing/InputtingWindow.hpp
		windowing/InputtingWindow.cpp
        windowing/Windowing.hpp
		windowing/Windowing.cpp
        windowing/WindowSettings.hpp)

set(OPEN_DEMO_SRC
        ${OPEN_DEMO_SRC_COMMON}
        ${OPEN_DEMO_SRC_PLATFORM}
        ${OPEN_DEMO_SRC_MODULE_FILE_SYSTEM}
        ${OPEN_DEMO_SRC_MODULE_INPUTTING}
        ${OPEN_DEMO_SRC_MODULE_WINDOWING}
        )

add_executable(open_demo WIN32 ${OPEN_DEMO_SRC})
target_link_libraries(open_demo ${OPEN_DEMO_LINK_LIBRARIES})
target_compile_definitions(open_demo PRIVATE ${OPEN_DEMO_DEFINITIONS})
target_include_directories(open_demo PRIVATE ${OPEN_DEMO_INCLUDE_DIRS})

if(MSVC)
	set_target_properties(open_demo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build/build/bin/")
    target_compile_options(open_demo PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(open_demo PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)