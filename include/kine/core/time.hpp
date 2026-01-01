#pragma once

namespace kine::time
{

inline float dt = 0.0f;
inline float accumulator = 0.0f;
inline float fixed_dt = 1.0f / 60.0f;
inline float alpha = 0.0f;

void begin_frame();

inline float last_frame_time = 0.0f;

}  // namespace kine::time
