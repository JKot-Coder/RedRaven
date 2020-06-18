project (common)

set(COMMON_SRC
        VecMath.h
        Common.hpp
        Logger.hpp
        Logger.cpp
        Exception.cpp
        Exception.hpp
        Stream.hpp
        Time.cpp
        Time.hpp)
        
add_library(${PROJECT_NAME} ${COMMON_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} fmt)