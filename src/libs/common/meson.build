src = files(
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
    'Math.cpp',
    'Math.hpp',
    'Math.hpp',
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
    'VecMath.h',
)

src += files(
    'debug/Debug.cpp',
    'debug/Debug.hpp',
    'debug/DebugStream.cpp',
    'debug/DebugStream.hpp',
    'debug/LeakDetector.cpp',
    'debug/LeakDetector.hpp',
    'debug/Logger.cpp',
    'debug/Logger.hpp',
)

utfcpp_dep = dependency('utfcpp', fallback: ['utfcpp', 'utfcpp_dep'])
fmt_dep = dependency('fmt', fallback: ['fmt', 'fmt_dep'])

common_deps = [
    utfcpp_dep,
    fmt_dep,
]

common_inc = include_directories('..')

common_lib = static_library(
    'common',
    src,
    install: true,
    include_directories: common_inc,
    dependencies: common_deps,
    cpp_pch: ['pch\pch.hpp', 'pch\pch.cpp'],
)
