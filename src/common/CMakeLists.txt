project (common)

SET( THREADING_SRC
	threading/AccessGuard.hpp
	threading/BufferedChannel.hpp
    threading/Thread.hpp
)
SOURCE_GROUP( "Threading" FILES ${THREADING_SRC} )

SET( DEBUGGING_SRC
	debugging/Logger.hpp
    debugging/Logger.cpp
)
SOURCE_GROUP( "Debugging" FILES ${DEBUGGING_SRC} )

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
        ${DEBUGGING_SRC}
        ${BACKWARD_ENABLE}
)
        
add_library(${PROJECT_NAME} ${COMMON_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} fmt utf8cpp)
target_precompile_headers(${PROJECT_NAME} PUBLIC pch.hpp)

add_backward(${PROJECT_NAME})