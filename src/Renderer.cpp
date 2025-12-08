#include "Renderer.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* displayFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main() {
    FragColor = texture(screenTexture, TexCoord);
}
)";

const char* raytracingFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform vec2 resolution;
uniform vec3 cameraPos;
uniform vec3 cameraLowerLeft;
uniform vec3 cameraHorizontal;
uniform vec3 cameraVertical;

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    vec3 material;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform Sphere spheres[5];
uniform int sphereCount;
uniform Light light;
uniform vec3 backgroundColor;

struct HitRecord {
    float t;
    vec3 point;
    vec3 normal;
    vec3 color;
    vec3 material;
    float shininess;
    bool hit;
};

HitRecord intersectSphere(vec3 origin, vec3 direction, Sphere sphere) {
    HitRecord hit;
    hit.hit = false;

    vec3 oc = origin - sphere.center;
    float a = dot(direction, direction);
    float b = 2.0 * dot(oc, direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) return hit;

    float t = (-b - sqrt(discriminant)) / (2.0 * a);
    if (t < 0.001) {
        t = (-b + sqrt(discriminant)) / (2.0 * a);
        if (t < 0.001) return hit;
    }

    hit.hit = true;
    hit.t = t;
    hit.point = origin + direction * t;
    hit.normal = normalize(hit.point - sphere.center);
    hit.color = sphere.color;
    hit.material = sphere.material;
    hit.shininess = sphere.shininess;

    return hit;
}

HitRecord intersectScene(vec3 origin, vec3 direction) {
    HitRecord closest;
    closest.hit = false;
    closest.t = 1000.0;

    for (int i = 0; i < sphereCount; i++) {
        HitRecord hit = intersectSphere(origin, direction, spheres[i]);
        if (hit.hit && hit.t < closest.t) {
            closest = hit;
        }
    }

    return closest;
}

bool isInShadow(vec3 point, vec3 lightPos) {
    vec3 lightDir = normalize(lightPos - point);
    float lightDist = length(lightPos - point);
    HitRecord shadowHit = intersectScene(point + lightDir * 0.001, lightDir);
    return shadowHit.hit && shadowHit.t < lightDist;
}

vec3 computePhongLighting(HitRecord hit, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - hit.point);
    vec3 reflectDir = reflect(-lightDir, hit.normal);

    vec3 ambient = hit.material.x * hit.color;

    if (isInShadow(hit.point, light.position)) {
        return ambient;
    }

    float diff = max(dot(hit.normal, lightDir), 0.0);
    vec3 diffuse = hit.material.y * diff * hit.color * light.color * light.intensity;

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), hit.shininess);
    vec3 specular = hit.material.z * spec * light.color * light.intensity;

    return ambient + diffuse + specular;
}

void main() {
    vec2 uv = TexCoord;

    vec3 rayOrigin = cameraPos;
    vec3 rayDir = normalize(cameraLowerLeft + cameraHorizontal * uv.x +
                           cameraVertical * uv.y - cameraPos);

    HitRecord hit = intersectScene(rayOrigin, rayDir);

    vec3 color;
    if (hit.hit) {
        vec3 viewDir = normalize(rayOrigin - hit.point);
        color = computePhongLighting(hit, viewDir);
    } else {
        color = backgroundColor;
    }

    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}
)";

const char* computeShaderSource = R"(
#version 430 core
layout (local_size_x = 8, local_size_y = 8) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;

uniform vec2 resolution;
uniform vec3 cameraPos;
uniform vec3 cameraLowerLeft;
uniform vec3 cameraHorizontal;
uniform vec3 cameraVertical;

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    vec3 material;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform Sphere spheres[5];
uniform int sphereCount;
uniform Light light;
uniform vec3 backgroundColor;

struct HitRecord {
    float t;
    vec3 point;
    vec3 normal;
    vec3 color;
    vec3 material;
    float shininess;
    bool hit;
};

HitRecord intersectSphere(vec3 origin, vec3 direction, Sphere sphere) {
    HitRecord hit;
    hit.hit = false;

    vec3 oc = origin - sphere.center;
    float a = dot(direction, direction);
    float b = 2.0 * dot(oc, direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) return hit;

    float t = (-b - sqrt(discriminant)) / (2.0 * a);
    if (t < 0.001) {
        t = (-b + sqrt(discriminant)) / (2.0 * a);
        if (t < 0.001) return hit;
    }

    hit.hit = true;
    hit.t = t;
    hit.point = origin + direction * t;
    hit.normal = normalize(hit.point - sphere.center);
    hit.color = sphere.color;
    hit.material = sphere.material;
    hit.shininess = sphere.shininess;

    return hit;
}

HitRecord intersectScene(vec3 origin, vec3 direction) {
    HitRecord closest;
    closest.hit = false;
    closest.t = 1000.0;

    for (int i = 0; i < sphereCount; i++) {
        HitRecord hit = intersectSphere(origin, direction, spheres[i]);
        if (hit.hit && hit.t < closest.t) {
            closest = hit;
        }
    }

    return closest;
}

bool isInShadow(vec3 point, vec3 lightPos) {
    vec3 lightDir = normalize(lightPos - point);
    float lightDist = length(lightPos - point);
    HitRecord shadowHit = intersectScene(point + lightDir * 0.001, lightDir);
    return shadowHit.hit && shadowHit.t < lightDist;
}

vec3 computePhongLighting(HitRecord hit, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - hit.point);
    vec3 reflectDir = reflect(-lightDir, hit.normal);

    vec3 ambient = hit.material.x * hit.color;

    if (isInShadow(hit.point, light.position)) {
        return ambient;
    }

    float diff = max(dot(hit.normal, lightDir), 0.0);
    vec3 diffuse = hit.material.y * diff * hit.color * light.color * light.intensity;

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), hit.shininess);
    vec3 specular = hit.material.z * spec * light.color * light.intensity;

    return ambient + diffuse + specular;
}

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(imgOutput);

    if (pixelCoords.x >= dims.x || pixelCoords.y >= dims.y) return;

    vec2 uv = vec2(pixelCoords) / vec2(dims);

    vec3 rayOrigin = cameraPos;
    vec3 rayDir = normalize(cameraLowerLeft + cameraHorizontal * uv.x +
                           cameraVertical * uv.y - cameraPos);

    HitRecord hit = intersectScene(rayOrigin, rayDir);

    vec3 color;
    if (hit.hit) {
        vec3 viewDir = normalize(rayOrigin - hit.point);
        color = computePhongLighting(hit, viewDir);
    } else {
        color = backgroundColor;
    }

    color = pow(color, vec3(1.0/2.2));
    imageStore(imgOutput, pixelCoords, vec4(color, 1.0));
}
)";

Renderer::Renderer(int width, int height, bool useComputeShader)
        : width(width), height(height), useComputeShader(useComputeShader) {

    setupTexture();
    setupQuad();

    if (useComputeShader) {
        computeProgram = createComputeProgram(computeShaderSource);
        fragmentProgram = createProgram(vertexShaderSource, displayFragmentShaderSource);
    } else {
        fragmentProgram = createProgram(vertexShaderSource, raytracingFragmentShaderSource);
    }
}

Renderer::~Renderer() {
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    if (useComputeShader) glDeleteProgram(computeProgram);
    glDeleteProgram(fragmentProgram);
}

void Renderer::setupQuad() {
    float quadVertices[] = {
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void Renderer::setupTexture() {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

GLuint Renderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

GLuint Renderer::createProgram(const char* vertSource, const char* fragSource) {
    GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertSource);
    GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    return program;
}

GLuint Renderer::createComputeProgram(const char* compSource) {
    GLuint compShader = compileShader(GL_COMPUTE_SHADER, compSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, compShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Compute program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(compShader);
    return program;
}

void Renderer::uploadSceneData(const Scene& scene, const Camera& camera) {
    GLuint program = useComputeShader ? computeProgram : fragmentProgram;
    glUseProgram(program);

    glUniform2f(glGetUniformLocation(program, "resolution"), width, height);
    glUniform3f(glGetUniformLocation(program, "cameraPos"),
                camera.position.x, camera.position.y, camera.position.z);
    glUniform3f(glGetUniformLocation(program, "cameraLowerLeft"),
                camera.lowerLeftCorner.x, camera.lowerLeftCorner.y, camera.lowerLeftCorner.z);
    glUniform3f(glGetUniformLocation(program, "cameraHorizontal"),
                camera.horizontal.x, camera.horizontal.y, camera.horizontal.z);
    glUniform3f(glGetUniformLocation(program, "cameraVertical"),
                camera.vertical.x, camera.vertical.y, camera.vertical.z);

    glUniform1i(glGetUniformLocation(program, "sphereCount"), scene.spheres.size());
    for (size_t i = 0; i < scene.spheres.size() && i < 5; i++) {
        std::string base = "spheres[" + std::to_string(i) + "]";
        glUniform3f(glGetUniformLocation(program, (base + ".center").c_str()),
                    scene.spheres[i].center.x, scene.spheres[i].center.y, scene.spheres[i].center.z);
        glUniform1f(glGetUniformLocation(program, (base + ".radius").c_str()),
                    scene.spheres[i].radius);
        glUniform3f(glGetUniformLocation(program, (base + ".color").c_str()),
                    scene.spheres[i].material.color.x,
                    scene.spheres[i].material.color.y,
                    scene.spheres[i].material.color.z);
        glUniform3f(glGetUniformLocation(program, (base + ".material").c_str()),
                    scene.spheres[i].material.ambient,
                    scene.spheres[i].material.diffuse,
                    scene.spheres[i].material.specular);
        glUniform1f(glGetUniformLocation(program, (base + ".shininess").c_str()),
                    scene.spheres[i].material.shininess);
    }

    if (!scene.lights.empty()) {
        glUniform3f(glGetUniformLocation(program, "light.position"),
                    scene.lights[0].position.x, scene.lights[0].position.y, scene.lights[0].position.z);
        glUniform3f(glGetUniformLocation(program, "light.color"),
                    scene.lights[0].color.x, scene.lights[0].color.y, scene.lights[0].color.z);
        glUniform1f(glGetUniformLocation(program, "light.intensity"),
                    scene.lights[0].intensity);
    }

    glUniform3f(glGetUniformLocation(program, "backgroundColor"),
                scene.backgroundColor.x, scene.backgroundColor.y, scene.backgroundColor.z);
}

void Renderer::render(const Scene& scene, const Camera& camera) {
    if (useComputeShader) {
        glUseProgram(computeProgram);
        uploadSceneData(scene, camera);

        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute((width + 7) / 8, (height + 7) / 8, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glUseProgram(fragmentProgram);
        glUniform1i(glGetUniformLocation(fragmentProgram, "screenTexture"), 0);
    } else {
        glUseProgram(fragmentProgram);
        uploadSceneData(scene, camera);
    }

    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::resize(int width, int height) {
    this->width = width;
    this->height = height;
    glDeleteTextures(1, &texture);
    setupTexture();
}

Vector3 Renderer::traceRay(const Ray& ray, const Scene& scene, int depth) {
    if (depth > 3) return scene.backgroundColor;

    HitRecord hit = scene.intersect(ray);
    if (!hit.hit) return scene.backgroundColor;

    Vector3 viewDir = (ray.origin - hit.point).normalize();
    Vector3 color(0, 0, 0);

    color = color + hit.material.color * hit.material.ambient;

    for (const auto& light : scene.lights) {
        if (scene.isInShadow(hit.point, light.position)) {
            continue;
        }

        Vector3 lightDir = (light.position - hit.point).normalize();
        Vector3 reflectDir = lightDir.reflect(hit.normal) * -1.0f;

        float diffuseIntensity = std::max(0.0f, hit.normal.dot(lightDir));
        Vector3 diffuse = hit.material.color * hit.material.diffuse *
                          diffuseIntensity * light.color * light.intensity;

        float specularIntensity = std::pow(std::max(0.0f, viewDir.dot(reflectDir)),
                                           hit.material.shininess);
        Vector3 specular = light.color * hit.material.specular *
                           specularIntensity * light.intensity;

        color = color + diffuse + specular;
    }

    return color;
}

void Renderer::renderCPU(const Scene& scene, const Camera& camera,
                         std::vector<unsigned char>& pixels) {
    pixels.resize(width * height * 3);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float u = float(x) / float(width);
            float v = float(height - 1 - y) / float(height);

            Ray ray = camera.getRay(u, v);
            Vector3 color = traceRay(ray, scene);

            color.x = std::pow(color.x, 1.0f / 2.2f);
            color.y = std::pow(color.y, 1.0f / 2.2f);
            color.z = std::pow(color.z, 1.0f / 2.2f);

            color.x = std::max(0.0f, std::min(1.0f, color.x));
            color.y = std::max(0.0f, std::min(1.0f, color.y));
            color.z = std::max(0.0f, std::min(1.0f, color.z));

            int idx = (y * width + x) * 3;
            pixels[idx + 0] = static_cast<unsigned char>(color.x * 255);
            pixels[idx + 1] = static_cast<unsigned char>(color.y * 255);
            pixels[idx + 2] = static_cast<unsigned char>(color.z * 255);
        }
    }
}