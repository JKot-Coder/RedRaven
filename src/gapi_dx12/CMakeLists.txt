project (gapi_dx12)

set(GAPI_SRC
        pch.hpp
        d3dx12.h
        Config.hpp
        ComSharedPtr.hpp
        Device.hpp
        Device.cpp
        D3DUtils.hpp
        D3DUtils.cpp
        FenceImpl.hpp
        FenceImpl.cpp
        FencedRingBuffer.hpp
        CommandListImpl.hpp
        CommandListImpl.cpp)
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
target_link_libraries(${PROJECT_NAME} common gapi d3d12.lib dxgi.lib dxguid.lib WindowsApp.lib)
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)