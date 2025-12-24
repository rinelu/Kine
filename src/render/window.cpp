#include "window.hpp"
#include <cstdlib>
#include "log.hpp"

namespace kine
{

Window::Window(int width, int height, const char* title)
{
    LOG_INFO("Window: initializing GLFW");
    if (!glfwInit())
    {
        LOG_ERROR("Window: failed to initialize GLFW");
        std::abort();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    LOG_INFO("Window: creating GLFW window");
    if (!window)
    {
        LOG_ERROR("Window failed to create GLFW window");
        glfwTerminate();
        std::abort();
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // vsync

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        LOG_ERROR("Window: Failed to load GLAD");
        std::abort();
    }

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, update_viewport);
}

void Window::update_viewport(GLFWwindow*, int width, int height) { glViewport(0, 0, width, height); }
}  // namespace kine
