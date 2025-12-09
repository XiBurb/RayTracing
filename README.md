Simple implementation of Ray Tracing using OpenGL with c++

Assemble wtih CMake:

mkdir build cd build

cmake .. -DCMAKE_BUILD_TYPE=Release

cmake --build . --config Release

Project Dependencies(must be placed in project in this way: CompGraph/includes/ -- each library in their directory): GLFW 3.3+ Window creation and input management (mouse, keyboard) GLAD - OpenGL function loader KHR - OpenGL compatibility headers

OpenGL 4.3+ Graphics API for rendering and shaders