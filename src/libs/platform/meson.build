platform_src = files(
    'Input.cpp',
    'Input.hpp',
    'Window.hpp',
    'Toolkit.hpp',
    'Toolkit.cpp',
)

platform_src += files(
    'glfw/GlfwWindow.cpp',
    'glfw/GlfwWindow.hpp',
    'glfw/GlfwToolkit.cpp',
    'glfw/GlfwToolkit.hpp',
)

glfw_dep = dependency('glfw', fallback: ['glfw', 'glfw_dep'], default_options : ['install=true'])

platform_lib = static_library(
    'platform',
    platform_src,
    install: true,
    include_directories: include_directories('..'),
    dependencies: [common_dep, glfw_dep],
    cpp_pch: ['pch\pch.hpp', 'pch\pch.cpp'],
)

platform_dep = declare_dependency(
    include_directories: include_directories('..'),
    dependencies: [common_dep, glfw_dep],
    link_with: platform_lib,
)