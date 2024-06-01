subdir('editor_ecs')

editor_src = files(
    'Application.cpp',
    'Application.hpp',
    'main.cpp',
)

editor_src += files(
     'imgui_impl/ImguiPlatformImpl.cpp',
     'imgui_impl/ImguiPlatformImpl.hpp',
#     'imgui_impl/ImguiRenderImpl.cpp',
#     'imgui_impl/ImguiRenderImpl.hpp',
)

imgui_dep = dependency('imgui', fallback: ['imgui', 'imgui_dep'])

executable('editor',
  editor_src,
  dependencies: [common_dep, render_dep, gapi_dx12_dep, imgui_dep, platform_dep],
  install : true,
  win_subsystem: 'windows',
  cpp_pch: ['pch\pch.hpp', 'pch\pch.cpp']
 )