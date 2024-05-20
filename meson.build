project('RedRaven', 'cpp', default_options : ['werror=true', 'warning_level=3', 'cpp_std=c++17'])

if get_option('buildtype').startswith('debug')
  add_project_arguments('-DDEBUG', language : 'cpp')
endif

if not meson.get_compiler('cpp').get_id().startswith('msvc')
  add_project_arguments('-fms-extensions', language: 'cpp')
  add_project_arguments('-fms-compatibility', language: 'cpp')
  add_project_arguments('-Wno-language-extension-token', language: 'cpp')
  add_project_arguments('-Wno-nested-anon-types', language: 'cpp') 
  add_project_arguments('-Wno-unused-function', language: 'cpp')
endif

if target_machine.system().startswith('windows')
  add_project_arguments('-DOS_WINDOWS', language : 'cpp')
  add_project_arguments('-DNOMINMAX', language : 'cpp')
  add_project_arguments('-DUNICODE', language : 'cpp')
endif
#get_option('b_sanitize').startswith('address') and meson.get_compiler('cpp').get_id().startswith('clang')

if get_option('buildtype').startswith('debug') 
  add_project_arguments(['-fsanitize=address'], language: 'cpp')
  add_project_arguments(['-fsanitize=address'], language: 'c')
  add_project_link_arguments(['-fsanitize=address'], language: 'cpp')
  add_project_link_arguments(['-fsanitize=address'], language: 'c')
  add_project_arguments(['-fno-omit-frame-pointer'], language: 'cpp')
  add_project_arguments(['-fno-omit-frame-pointer'], language: 'c')
endif

subdir('src')