import sys
import os
import clang.cindex

index = clang.cindex.Index.create()
translation_unit = index.parse(sys.argv[1], ['-x', 'c++', '-std=c++17', '-D__CODE_GENERATOR__'])