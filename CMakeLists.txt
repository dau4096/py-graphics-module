cmake_minimum_required(VERSION 3.15)
project(gl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(BUILD_SHARED_LIBS OFF)

find_package(pybind11 REQUIRED)

pybind11_add_module(gl
    gl.cpp
    src/graphics.cpp
)

if(WIN32) #Windows versions
    set(GLFW_ROOT "${CMAKE_SOURCE_DIR}/third_party/glfw")
    set(GLEW_ROOT "${CMAKE_SOURCE_DIR}/third_party/glew")
    set(GLM_ROOT  "${CMAKE_SOURCE_DIR}/third_party/glm")

    target_include_directories(gl PRIVATE
        "${GLFW_ROOT}/include"
        "${GLEW_ROOT}/include"
        "${GLM_ROOT}"
    )

    target_link_libraries(gl PRIVATE
        "${GLFW_ROOT}/lib-vc2022/glfw3.lib"
        "${GLEW_ROOT}/lib/Release/x64/glew32s.lib"
        OpenGL::GL
    )

    target_compile_definitions(gl PRIVATE GLEW_STATIC)

else() #Linux/MacOS
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(OpenGL REQUIRED)

    target_link_libraries(gl PRIVATE glfw GLEW::GLEW OpenGL::GL)
endif()
