cmake_minimum_required(VERSION 3.15)
project(gl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)


#pybind11 via vcpkg (windows) or system (linux)
find_package(pybind11 REQUIRED)


#Pybind11 module
set(PYBIND11_PYTHON_VERSION 3.12)
set(PYBIND11_FINDPYTHON ON)
pybind11_add_module(gl
    gl.cpp
    src/graphics.cpp
)


#OS specific library dependencies
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)


#Include headers
target_include_directories(gl PRIVATE glm)
target_include_directories(gl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


#Link libraries
target_link_libraries(gl PRIVATE
    glfw
    GLEW::GLEW
    OpenGL::GL
)

#Req. for static GLEW
target_compile_definitions(gl PRIVATE GLEW_STATIC)
