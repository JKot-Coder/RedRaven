project (render)

set(Render_SRC
      RenderContext.cpp
      RenderContext.hpp
      Submission.hpp
      Submission.cpp)
        
add_library(${PROJECT_NAME} ${Render_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} common)