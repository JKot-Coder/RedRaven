set( IO_SRC
    io/File.hpp
    io/FileSystem.hpp
    io/OSFileSystem.hpp
    io/OSFileSystem.cpp
    PARENT_SCOPE
)

set( IO_POSIX_SRC
    io/platform/posix/File.cpp)

if ( UNIX OR APPLE )
    SET( OS_POSIX TRUE )
endif()

if ( OS_POSIX )
    set( IO_SRC ${IO_SRC} ${IO_POSIX_SRC} PARENT_SCOPE )
endif()

source_group( "IO" FILES ${IO_SRC} )