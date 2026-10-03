add_library(glad STATIC
    src/glad.c
)

target_include_directories(glad PUBLIC
    include
)

target_link_libraries(glad PUBLIC
    OpenGL::GL
    ${CMAKE_DL_LIBS}
)
