asset_test_inc = [
  common_inc,
]

asset_test_deps = [
  common_deps,
]

executable('asset_test',
 'main.cpp',
  include_directories: asset_test_inc,
  dependencies: [asset_test_deps],
  install : true,
  cpp_pch: ['pch\pch.hpp', 'pch\pch.cpp']
 )