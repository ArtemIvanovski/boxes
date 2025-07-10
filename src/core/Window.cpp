#include "Window.h"
#include <glad/glad.h>  // ВАЖНО: glad должен быть включен перед GLFW
#include <iostream>
#include <stdexcept>

Window::Window(int width, int height, const std::string& title)
    : width(width), height(height), title(title) {

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    // Set user pointer for callbacks
    glfwSetWindowUserPointer(window, this);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallbackStatic);
    glfwSetScrollCallback(window, scrollCallbackStatic);
    glfwSetKeyCallback(window, keyCallbackStatic);

    // Enable OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, width, height);
}

Window::~Window() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    glfwSwapBuffers(window);
}

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

// Static callback functions
void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    win->width = width;
    win->height = height;
    glViewport(0, 0, width, height);

    if (win->resizeCallback) {
        win->resizeCallback(width, height);
    }
}

void Window::mouseCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win->mouseCallback) {
        win->mouseCallback(xpos, ypos);
    }
}

void Window::scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win->scrollCallback) {
        win->scrollCallback(xoffset, yoffset);
    }
}

void Window::keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win->keyCallback) {
        win->keyCallback(key, scancode, action, mods);
    }
}