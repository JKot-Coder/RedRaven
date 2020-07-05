project (common)

set(COMMON_SRC
        pch.hpp
        Math.hpp   
        Math.cpp
        Config.hpp
        VecMath.h
        Check.cpp
        NativeWindowHandle.hpp
        String.hpp
        Logger.hpp
        Logger.cpp
        Exception.hpp
        Stream.hpp
        Time.cpp
        Time.hpp)
        
add_library(${PROJECT_NAME} ${COMMON_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} fmt utf8cpp eigen)
target_precompile_headers(${PROJECT_NAME} PUBLIC pch.hpp)