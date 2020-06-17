project (common)

set(OPEN_DEMO_SRC_MODULE_COMMON
        VecMath.h
        Utils.hpp
        Exception.cpp
        Exception.hpp
        Stream.hpp
        Time.cpp
        Time.hpp)
        
add_library(${PROJECT_NAME} ${OPEN_DEMO_SRC_MODULE_COMMON})