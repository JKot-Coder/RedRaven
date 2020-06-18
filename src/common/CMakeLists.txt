project (common)

set(COMMON_SRC
        VecMath.h
        Utils.hpp
        Exception.cpp
        Exception.hpp
        Stream.hpp
        Time.cpp
        Time.hpp)
        
add_library(${PROJECT_NAME} ${COMMON_SRC})