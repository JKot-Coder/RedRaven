project (render)

set(Render_SRC
      RenderContext.cpp
      RenderContext.hpp
      Submission.hpp
      Submission.cpp)
        
add_library(${PROJECT_NAME} ${Render_SRC})
target_link_libraries(${PROJECT_NAME} common)