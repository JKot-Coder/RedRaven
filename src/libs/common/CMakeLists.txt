project (common)

set( THREADING_SRC
	threading/AccessGuard.hpp
	threading/BufferedChannel.hpp
    threading/Thread.hpp
    threading/Event.hpp
    threading/Mutex.hpp
    threading/ConditionVariable.hpp
    threading/SpinLock.hpp
)
source_group( "Threading" FILES ${THREADING_SRC} )

set( DEBUG_SRC
	debug/Logger.hpp
    debug/Logger.cpp
    debug/DebugStream.hpp
    debug/DebugStream.cpp
    debug/LeakDetector.hpp
    debug/LeakDetector.cpp
    debug/Debug.hpp
    debug/Debug.cpp
)
source_group( "Debug" FILES ${DEBUG_SRC} )

set(IO_SRC)
add_subdirectory(io) # IO_SRC

set( COMMON_SRC
    Check.cpp
    ComPtr.hpp
    Config.hpp
    DataBuffer.hpp
    EnumClassOperators.hpp
    Event.cpp
    Event.hpp
    EventProvider.hpp
    Exception.hpp
    LinearAllocator.cpp
    LinearAllocator.hpp
    LinearAllocator.inl
    Math.cpp
    Math.hpp  
    NonCopyableMovable.hpp
    OnScopeExit.hpp
    pch.hpp
    RingBuffer.hpp
    RingQueue.hpp
    Result.hpp
    Result.cpp
    Singleton.hpp
    Stream.hpp
    String.hpp
    Time.cpp
    Time.hpp
    VecMath.h
)
source_group( "" FILES ${COMMON_SRC} )

set( COMMON_SRC
    ${COMMON_SRC}
    ${THREADING_SRC}
    ${DEBUG_SRC}
    ${IO_SRC}
)
        
add_library(${PROJECT_NAME} ${COMMON_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/..)
target_link_libraries(${PROJECT_NAME} fmt utf8cpp backward_object)
target_precompile_headers(${PROJECT_NAME} PUBLIC pch.hpp)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)