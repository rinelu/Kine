#include "time.hpp"

#include <algorithm>

namespace kine
{

void Time::begin_frame()
{
    // dt = GetFrameTime();
    accumulator += dt;
    alpha = accumulator / fixed_dt;
    alpha = std::clamp(alpha, 0.0f, 1.0f);
}

}  // namespace kine
