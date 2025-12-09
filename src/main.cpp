#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <iostream>
#include <vector>

#include "Vector3.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "ImageUtils.hpp"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const char* WINDOW_TITLE = "GPU Ray Tracer";

Scene scene;
Camera camera;
Renderer* renderer = nullptr;
bool useComputeShader = true;

struct CameraController {
    float radius = 5.0f;
    float theta = 0.0f;
    float phi = 60.0f;
    Vector3 target = Vector3(0, 0, 0);

    bool isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    void updateCamera(Camera& cam) {
        float thetaRad = theta * 3.14159265359f / 180.0f;
        float phiRad = phi * 3.14159265359f / 180.0f;

        float x = radius * sin(phiRad) * cos(thetaRad);
        float y = radius * cos(phiRad);
        float z = radius * sin(phiRad) * sin(thetaRad);

        Vector3 position = target + Vector3(x, y, z);

        cam = Camera(position, target, 45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT);
    }
} cameraController;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (renderer) {
        renderer->resize(width, height);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        useComputeShader = !useComputeShader;
        delete renderer;
        renderer = new Renderer(WINDOW_WIDTH, WINDOW_HEIGHT, useComputeShader);
        std::cout << "Switched to " << (useComputeShader ? "Compute" : "Fragment")
                  << " Shader mode" << std::endl;
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        std::cout << "Rendering screenshot..." << std::endl;
        std::vector<unsigned char> pixels;
        renderer->renderCPU(scene, camera, pixels);
        ImageUtils::saveImage(pixels, WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        cameraController.radius = 5.0f;
        cameraController.theta = 0.0f;
        cameraController.phi = 60.0f;
        cameraController.updateCamera(camera);
        std::cout << "Camera reset" << std::endl;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            cameraController.isDragging = true;
            glfwGetCursorPos(window, &cameraController.lastMouseX, &cameraController.lastMouseY);
        } else if (action == GLFW_RELEASE) {
            cameraController.isDragging = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (cameraController.isDragging) {
        double deltaX = xpos - cameraController.lastMouseX;
        double deltaY = ypos - cameraController.lastMouseY;

        float sensitivity = 0.5f;

        cameraController.theta += deltaX * sensitivity;
        cameraController.phi -= deltaY * sensitivity;

        if (cameraController.phi < 5.0f) cameraController.phi = 5.0f;
        if (cameraController.phi > 175.0f) cameraController.phi = 175.0f;

        while (cameraController.theta > 360.0f) cameraController.theta -= 360.0f;
        while (cameraController.theta < 0.0f) cameraController.theta += 360.0f;

        cameraController.updateCamera(camera);

        cameraController.lastMouseX = xpos;
        cameraController.lastMouseY = ypos;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraController.radius -= yoffset * 0.5f;

    if (cameraController.radius < 1.5f) cameraController.radius = 1.5f;
    if (cameraController.radius > 15.0f) cameraController.radius = 15.0f;

    cameraController.updateCamera(camera);
}

void setupScene() {
    scene.addSphere(Sphere(
            Vector3(0, -100.5, 0),
            100.0f,
            Material(Vector3(0.5f, 0.5f, 0.5f), 0.1f, 0.7f, 0.2f, 16.0f)
    ));

    scene.addSphere(Sphere(
            Vector3(0, 0, 0),
            0.5f,
            Material(Vector3(1.0f, 0.2f, 0.2f), 0.1f, 0.7f, 0.5f, 32.0f)
    ));

    scene.addSphere(Sphere(
            Vector3(-1.2, 0, 0),
            0.5f,
            Material(Vector3(0.2f, 1.0f, 0.2f), 0.1f, 0.6f, 0.4f, 16.0f)
    ));

    scene.addSphere(Sphere(
            Vector3(1.2, 0, 0),
            0.5f,
            Material(Vector3(0.2f, 0.2f, 1.0f), 0.1f, 0.8f, 0.6f, 64.0f)
    ));

    scene.addSphere(Sphere(
            Vector3(0, 0.8, 0),
            0.3f,
            Material(Vector3(1.0f, 1.0f, 0.2f), 0.1f, 0.7f, 0.7f, 128.0f)
    ));

    scene.addLight(Light(
            Vector3(3, 4, 2),
            Vector3(1, 1, 1),
            1.0f
    ));

    scene.backgroundColor = Vector3(0.5f, 0.7f, 1.0f);
}

void setupCamera() {
    //cameraController.updateCamera(camera);
    camera = Camera(
            Vector3(0, 1, 5),    // Позиция камеры
            Vector3(0, 0, 0),    // Смотрим в центр
            45.0f,               // Угол обзора
            (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT  // Соотношение сторон
    );
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                          WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cout << "OpenGL Version: " << major << "." << minor << std::endl;

    if (major < 4 || (major == 4 && minor < 3)) {
        std::cout << "Compute shaders not supported, using fragment shader mode" << std::endl;
        useComputeShader = false;
    }

    setupScene();
    setupCamera();

    renderer = new Renderer(WINDOW_WIDTH, WINDOW_HEIGHT, useComputeShader);

    std::cout << "\n=== Controls ===" << std::endl;
    std::cout << "LEFT MOUSE + DRAG - Rotate camera around scene" << std::endl;
    std::cout << "SCROLL WHEEL      - Zoom in/out" << std::endl;
    std::cout << "SPACE             - Switch between Compute and Fragment Shader" << std::endl;
    std::cout << "S                 - Save screenshot (output/*.bmp)" << std::endl;
    std::cout << "R                 - Reset camera position" << std::endl;
    std::cout << "ESC               - Exit" << std::endl;
    std::cout << "===============\n" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer->render(scene, camera);

        glfwSwapBuffers(window);
    }

    delete renderer;
    glfwTerminate();

    return 0;
}