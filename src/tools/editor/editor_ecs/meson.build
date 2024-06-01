editor_ecs_src = files(
    'main.cpp'
)

flecs_dep = dependency('flecs', fallback: ['flecs', 'flecs_dep'])

shared_library('editor_ecs',
  editor_ecs_src,
  dependencies: [flecs_dep],
  install : true
 )