cmake_minimum_required (VERSION 3.2.2)
project(OpenDemo)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set (CMAKE_CXX_STANDARD 11)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    make_directory(${CMAKE_CURRENT_BINARY_DIR}/src/resources)
    file (GLOB_RECURSE resources "resources/*.*")

    foreach(resource ${resources})
        get_filename_component(filename ${resource} NAME)
        get_filename_component(dir ${resource} DIRECTORY)
        get_filename_component(dirname ${dir} NAME)

        set (output "")

        while(NOT ${dirname} STREQUAL resources)
            get_filename_component(path_component ${dir} NAME)
            set (output "${path_component}/${output}")
            get_filename_component(dir ${dir} DIRECTORY)
            get_filename_component(dirname ${dir} NAME)
        endwhile()

        set(output "${CMAKE_CURRENT_BINARY_DIR}/src/resources/${output}/${filename}")

        add_custom_command(
                COMMENT "Moving updated resource-file '${filename}'"
                OUTPUT ${output}
                DEPENDS ${resource}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${resource}
                ${output}
        )
        add_custom_target(${filename} ALL DEPENDS ${resource} ${output})

    endforeach()
endif()

add_subdirectory(src)