cmake_minimum_required(VERSION 3.14)
project(RayTracer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Проверяем наличие GLFW
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/includes/GLFW/include/GLFW/glfw3.h")
    message(FATAL_ERROR "GLFW not found! Expected: includes/GLFW/include/GLFW/glfw3.h")
endif()

# Ищем GLFW библиотеку в разных местах
find_library(GLFW_LIBRARY
        NAMES glfw3 glfw glfw3dll
        PATHS
        "${CMAKE_SOURCE_DIR}/includes/GLFW/lib"
        "${CMAKE_SOURCE_DIR}/includes/GLFW/lib-mingw-w64"
        "${CMAKE_SOURCE_DIR}/includes/GLFW/lib-vc2022"
        NO_DEFAULT_PATH
)

if(NOT GLFW_LIBRARY)
    message(FATAL_ERROR "GLFW library not found! Check includes/GLFW/ folder")
else()
    message(STATUS "Found GLFW library: ${GLFW_LIBRARY}")
endif()

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

# GLAD
add_library(glad STATIC "${CMAKE_SOURCE_DIR}/includes/glad/glad.c")
target_include_directories(glad PUBLIC "${CMAKE_SOURCE_DIR}/includes")

# Подключаем заголовки
target_include_directories(RayTracer PRIVATE
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/includes"
        "${CMAKE_SOURCE_DIR}/includes/GLFW/include"
)

# ЛИНКУЕМ библиотеки - это ключевое!
target_link_libraries(RayTracer
        glad           # GLAD библиотека
        ${GLFW_LIBRARY} # GLFW библиотека
        opengl32       # OpenGL для Windows
        gdi32          # GDI для Windows
)

# Для MinGW добавляем статическую линковку runtime
if(MINGW)
    target_link_options(RayTracer PRIVATE
            -static-libgcc
            -static-libstdc++
            -static
    )
endif()

# Настройки компилятора
target_compile_options(RayTracer PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Wno-unused-parameter
)