project (gapi)

set(GAPI_SRC
        Limits.hpp
        Resource.hpp
	    CommandList.hpp
        Fence.hpp
        FencedPool.hpp
        GAPIStatus.hpp
        GAPIStatus.cpp
        Device.hpp
        Device.cpp)
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
target_link_libraries(${PROJECT_NAME} common)