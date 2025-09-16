project (common)

set( THREADING_SRC
	threading/AccessGuard.hpp
	threading/BlockingRingQueue.hpp
    threading/Thread.hpp
    threading/Thread.cpp
    threading/Event.hpp
    threading/Mutex.hpp
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

set( HASHING_SRC
    hashing/Default.hpp
	hashing/Murmur.hpp
    hashing/Wyhash.hpp
)
source_group( "Hashing" FILES ${HASHING_SRC} )

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
    LinearAllocator.hpp
    ChunkAllocator.hpp
    NonCopyableMovable.hpp
    OnScopeExit.hpp
    pch/pch.hpp
    RingBuffer.hpp
    RingQueue.hpp
    Result.hpp
    Result.cpp
    Singleton.hpp
    Stream.hpp
    StringEncoding.hpp
    ErrorNo.cpp
    ErrorNo.hpp
    EastlAllocator.cpp
)
source_group( "" FILES ${COMMON_SRC} )

set( COMMON_SRC
    ${COMMON_SRC}
    ${THREADING_SRC}
    ${DEBUG_SRC}
    ${HASHING_SRC}
    ${IO_SRC}
)

add_library(${PROJECT_NAME}  ${COMMON_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/..)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt utf8cpp stl)
target_link_libraries(${PROJECT_NAME} PRIVATE RR::BuildSettings)
target_precompile_headers(${PROJECT_NAME} PUBLIC pch/pch.hpp)