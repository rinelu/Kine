#pragma once

namespace kine
{

class Time
{
   public:
    float dt = 0.0f;
    float accumulator = 0.0f;
    float fixed_dt = 1.0f / 60.0f;
    float alpha = 0.0f;

    void begin_frame();

   private:
    float last_frame_time = 0.0f;
};

}  // namespace kine
