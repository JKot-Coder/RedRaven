project (common)

SET( THREADING_SRC
	threading/AccessGuard.hpp
	threading/BufferedChannel.hpp
    threading/Thread.hpp
)
SOURCE_GROUP( "Threading" FILES ${THREADING_SRC} )

SET( DEBUG_SRC
	debug/Logger.hpp
    debug/Logger.cpp
    debug/DebugStream.hpp
    debug/DebugStream.cpp
)
SOURCE_GROUP( "Debug" FILES ${DEBUG_SRC} )

SET( COMMON_SRC
        pch.hpp
        Math.hpp  
        Math.cpp
        Config.hpp
        VecMath.h
        Check.cpp
        CircularBuffer.hpp
        NativeWindowHandle.hpp
        String.hpp
        Exception.hpp
        Stream.hpp
        Time.cpp
        Time.hpp
)
SOURCE_GROUP( "" FILES ${COMMON_SRC} )

set( COMMON_SRC
        ${COMMON_SRC}
        ${THREADING_SRC}
        ${DEBUG_SRC}
)
        
add_library(${PROJECT_NAME} ${COMMON_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} fmt utf8cpp backward_object)
target_precompile_headers(${PROJECT_NAME} PUBLIC pch.hpp)

add_backward(${PROJECT_NAME})