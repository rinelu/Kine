#include "core/time.hpp"

#include <algorithm>
#include "GLFW/glfw3.h"

namespace kine
{

// TODO: Maybe inline
void Time::begin_frame()
{
    float current_time = (float) glfwGetTime();
    dt = current_time - last_frame_time;
    last_frame_time = current_time;

    accumulator += dt;
    alpha = accumulator / fixed_dt;
    alpha = std::clamp(alpha, 0.0f, 1.0f);
}

}  // namespace kine
