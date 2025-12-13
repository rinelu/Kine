#include "window.hpp"
#include <cstdlib>
#include "log.hpp"

namespace kine
{

Window::Window(int width, int height, const char* title)
{
    if (!glfwInit())
    {
        LOG_ERROR("Failed to initialize GLFW");
        std::abort();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    LOG_INFO("Creating GLFW window");
    if (!window)
    {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        std::abort();
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // vsync

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        LOG_ERROR("Failed to load GLAD");
        std::abort();
    }

    glViewport(0, 0, width, height);
}
}  // namespace kine
