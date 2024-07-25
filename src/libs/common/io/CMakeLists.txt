set( IO_SRC
    io/File.hpp
    io/FileSystem.hpp
    io/FileSystem.cpp
)

if ( UNIX OR APPLE )
    set( IO_SRC ${IO_SRC}
    io/platform/posix/File.cpp
    PARENT_SCOPE )
else()
    set( IO_SRC ${IO_SRC}
    io/platform/windows/File.hpp
    io/platform/windows/File.cpp
    PARENT_SCOPE )
endif()

source_group( "IO" FILES ${IO_SRC} )