#pragma once
#include "GL.hpp"

namespace kine
{

class Window
{
   public:
    Window(int width, int height, const char* title);

    inline GLFWwindow* get() const { return window; }
    inline bool should_close() const { return glfwWindowShouldClose(window); }

   private:
    static void update_viewport(GLFWwindow* window, int width, int height);
    GLFWwindow* window;
};
}  // namespace kine
