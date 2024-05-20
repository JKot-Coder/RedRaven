render_src = files(
    'DeviceContext.cpp',
    'DeviceContext.hpp',
    'Submission.hpp',
    'Submission.cpp',
)
 
render_lib = static_library(
    'render',
    render_src,
    install: true,
    include_directories: include_directories('..'),
    dependencies: [common_dep, gapi_dep],
    cpp_pch: ['pch\pch.hpp', 'pch\pch.cpp'],
)

render_dep = declare_dependency(
    include_directories: include_directories('..'),
    dependencies: [common_dep, gapi_dep],
    link_with: render_lib,
)