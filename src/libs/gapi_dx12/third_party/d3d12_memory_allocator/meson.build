d3d12_memory_allocator_src = files(
    'D3D12MemAlloc.cpp',
    'D3D12MemAlloc.h',
)

d3d12_memory_allocator_lib = static_library(
    'd3d12_memory_allocator',
    d3d12_memory_allocator_src,
    override_options: ['werror=false', 'warning_level=0'],
    install: true
)

d3d12_memory_allocator_dep = declare_dependency(
    link_with: d3d12_memory_allocator_lib,
)
