common_src = files(
    'Check.cpp',
    'ComPtr.hpp',
    'Config.hpp',
    'DataBuffer.hpp',
    'EnumClassOperators.hpp',
    'ErrorNo.cpp',
    'ErrorNo.hpp',
    'Event.cpp',
    'Event.hpp',
    'EventProvider.hpp',
    'Exception.hpp',
    'LinearAllocator.cpp',
    'LinearAllocator.hpp',
    'NonCopyableMovable.hpp',
    'OnScopeExit.hpp',
    'Result.cpp',
    'Result.hpp',
    'RingBuffer.hpp',
    'RingQueue.hpp',
    'Singleton.hpp',
    'Stream.hpp',
    'String.hpp',
    'Time.cpp',
    'Time.hpp',
)

common_src += files(
    'debug/Debug.cpp',
    'debug/Debug.hpp',
    'debug/DebugStream.cpp',
    'debug/DebugStream.hpp',
    'debug/LeakDetector.cpp',
    'debug/LeakDetector.hpp',
    'debug/Logger.cpp',
    'debug/Logger.hpp',
)

common_src += files(
	'threading/AccessGuard.hpp',
	'threading/BufferedChannel.hpp',
    'threading/Thread.hpp',
    'threading/Event.hpp',
    'threading/Mutex.hpp',
    'threading/ConditionVariable.hpp',
    'threading/SpinLock.hpp',
)

utfcpp_dep = dependency('utfcpp', fallback: ['utfcpp', 'utfcpp_dep'])
fmt_dep = dependency('fmt', fallback: ['fmt', 'fmt_dep'])

common_deps = [
    utfcpp_dep,
    fmt_dep,
]

common_lib = static_library(
    'common',
    common_src,
    install: true,
    include_directories: include_directories('..'),
    dependencies: common_deps,
    cpp_pch: ['pch\pch.hpp', 'pch\pch.cpp'],
)

common_dep = declare_dependency(
    include_directories: include_directories('..'),
    dependencies: common_deps,
    link_with: common_lib,
)
