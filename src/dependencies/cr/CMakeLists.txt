project (cr)

add_library(${PROJECT_NAME} INTERFACE cr.h)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library("${PROJECT_NAME}_host" STATIC cr.h main.cpp)
target_include_directories("${PROJECT_NAME}_host" INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})