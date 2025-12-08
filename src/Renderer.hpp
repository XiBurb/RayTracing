#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "glad/glad.h"
#include <vector>
#include "Scene.hpp"
#include "Camera.hpp"

class Renderer {
private:
    int width;
    int height;
    GLuint computeProgram;
    GLuint fragmentProgram;
    GLuint texture;
    GLuint vao, vbo;
    bool useComputeShader;

    void setupQuad();
    void setupTexture();
    GLuint compileShader(GLenum type, const char* source);
    GLuint createProgram(const char* vertSource, const char* fragSource);
    GLuint createComputeProgram(const char* compSource);
    void uploadSceneData(const Scene& scene, const Camera& camera);

public:
    Renderer(int width, int height, bool useComputeShader = true);
    ~Renderer();

    void render(const Scene& scene, const Camera& camera);
    void resize(int width, int height);

    Vector3 traceRay(const Ray& ray, const Scene& scene, int depth = 0);
    void renderCPU(const Scene& scene, const Camera& camera,
                   std::vector<unsigned char>& pixels);
};

#endif