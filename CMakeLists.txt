cmake_minimum_required(VERSION 3.14)
project(RayTracer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
)

FetchContent_MakeAvailable(glfw glm)

set(SOURCES
        src/main.cpp
        src/Renderer.cpp
)

set(HEADERS
        src/Vector3.hpp
        src/Ray.hpp
        src/Sphere.hpp
        src/Camera.hpp
        src/Scene.hpp
        src/Renderer.hpp
        src/ImageUtils.hpp
)

add_executable(RayTracer ${SOURCES} ${HEADERS})

if(EXISTS ${CMAKE_SOURCE_DIR}/includes/glad/glad.c)
    message(STATUS "GLAD найден, добавляем в проект")
    add_library(glad ${CMAKE_SOURCE_DIR}/includes/glad/glad.c)
    target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/includes)
    target_link_libraries(RayTracer glad)
else()
    message(WARNING "GLAD НЕ НАЙДЕН! Создаем заглушку...")

    # Создаем временные файлы GLAD
    file(WRITE ${CMAKE_BINARY_DIR}/glad.h "#pragma once\nint gladLoadGLLoader(void*);")
    file(WRITE ${CMAKE_BINARY_DIR}/glad.c "#include \"glad.h\"\nint gladLoadGLLoader(void* p) { return 1; }")

    add_library(glad ${CMAKE_BINARY_DIR}/glad.c)
    target_include_directories(glad PUBLIC ${CMAKE_BINARY_DIR})
    target_link_libraries(RayTracer glad)
endif()

target_include_directories(RayTracer PRIVATE
        ${CMAKE_SOURCE_DIR}/includes
        ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(RayTracer
        glfw
        glm::glm
)

if(WIN32)
    target_link_libraries(RayTracer opengl32)
endif()

if(MSVC)
    target_compile_options(RayTracer PRIVATE /W4)
else()
    target_compile_options(RayTracer PRIVATE -Wall -Wextra -pedantic)
endif()