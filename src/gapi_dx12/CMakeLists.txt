project (gapi_dx12)

set(GAPI_SRC
        pch.hpp
        d3dx12.h
        Config.hpp
        ComSharedPtr.hpp
        D3DUtils.hpp
        D3DUtils.cpp
        Device.hpp
        Device.cpp
        FencedRingBuffer.hpp
        CommandListCompiler.hpp
        CommandListCompiler.cpp
        FenceImpl.hpp
        FenceImpl.cpp
        CommandListImpl.hpp
        CommandListImpl.cpp)
        
add_library(${PROJECT_NAME} ${GAPI_SRC})
target_link_libraries(${PROJECT_NAME} common gapi d3d12.lib dxgi.lib dxguid.lib WindowsApp.lib)
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)