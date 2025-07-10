#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

class Window {
private:
    GLFWwindow* window;
    int width, height;
    std::string title;

    // Event callbacks
    std::function<void(int, int)> resizeCallback;
    std::function<void(double, double)> mouseCallback;
    std::function<void(double, double)> scrollCallback;
    std::function<void(int, int, int, int)> keyCallback;

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseCallbackStatic(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();

    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;

    GLFWwindow* getGLFWWindow() const { return window; }

    // Event callback setters
    void setResizeCallback(std::function<void(int, int)> callback) { resizeCallback = callback; }
    void setMouseCallback(std::function<void(double, double)> callback) { mouseCallback = callback; }
    void setScrollCallback(std::function<void(double, double)> callback) { scrollCallback = callback; }
    void setKeyCallback(std::function<void(int, int, int, int)> callback) { keyCallback = callback; }
};

#endif // WINDOW_H