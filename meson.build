project('RedRaven', 'cpp', default_options : ['cpp_std=c++17'])

if get_option('buildtype').startswith('debug')
  add_project_arguments('-DDEBUG', language : 'cpp')
endif

if target_machine.system().startswith('windows')
  add_project_arguments('-DOS_WINDOWS', language : 'cpp')
endif

if get_option('b_sanitize').startswith('address') and meson.get_compiler('cpp').get_id().startswith('clang')
  #fix meson
  add_project_link_arguments([ '-fsanitize=address' ], language: 'cpp')
endif

subdir('src')
