editor_ecs_src = files(
    'main.cpp'
)

flecs_dep = dependency('flecs', fallback: ['flecs', 'flecs_dep'], default_options: ['build_example=disabled'])

editor_ecs_lib = shared_library('editor_ecs',
  editor_ecs_src,
  dependencies: [],
  install : true
 )

editor_ecs_dep = declare_dependency(
    dependencies: [],
    link_with: editor_ecs_lib,
)