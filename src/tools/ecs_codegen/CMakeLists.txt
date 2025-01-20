project(ecs_codegen)

set(SRC
    "main.cpp")
source_group("" FILES ${SRC})

set(SRC
    ${SRC})

set(LIBRARIES
    libclang)

if(NOT DEFINED ENV{LLVMInstallDir})
    message(FATAL_ERROR "LLVMInstallDir environment variable is not set!")
endif()

set(LLVM_DIR $ENV{LLVMInstallDir})
find_library(LIBCLANG_LIBRARY NAMES libclang libclang.lib PATHS "${LLVM_DIR}/lib" NO_DEFAULT_PATH)
message(WARNING "${LLVM_DIR}/include")

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC "${LLVM_DIR}/include")

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tools")