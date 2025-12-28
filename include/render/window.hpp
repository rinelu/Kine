#pragma once
#include "GL.hpp"

namespace kine::window
{

inline GLFWwindow* window;

void create(int width, int height, const char* title);

inline GLFWwindow* get() { return window; }
inline bool should_close() { return glfwWindowShouldClose(window); }

inline void update_viewport(GLFWwindow*, int width, int height) { glViewport(0, 0, width, height); };
}  // namespace kine::window
