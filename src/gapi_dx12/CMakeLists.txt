project (gapi_dx12)

set(GAPI_SRC
        ComSharedPtr.hpp
        Device.hpp
        Device.cpp)
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
target_link_libraries(${PROJECT_NAME} gapi)
target_include_directories("..")