set( IO_SRC
    File.hpp
    FileSystem.hpp
    OSFileSystem.hpp
    OSFileSystem.cpp
    PARENT_SCOPE
)

set( IO_POSIX_SRC 
    io/platform/posix/File.cpp)

if ( UNIX OR APPLE )
    SET( OS_POSIX TRUE )
endif()

#if ( OS_POSIX )
    set( IO_SRC ${IO_SRC} ${IO_POSIX_SRC} PARENT_SCOPE )
#endif()

source_group( "IO" FILES ${IO_SRC} )