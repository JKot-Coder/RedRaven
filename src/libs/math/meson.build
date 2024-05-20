math_src = files(
    'Math.cpp',
    'Math.hpp',
    'VecMath.h',
)

math_lib = static_library(
    'math',
    math_src,
    install: true,
    include_directories: include_directories('..'),
)

math_dep = declare_dependency(
    include_directories: include_directories('..'),
    link_with: math_lib,
)
