#pragma once

#include "kine/core/scheduler.hpp"
#include "kine/core/time.hpp"
#include "kine/flow/flow_tree.hpp"
#include "kine/io/input.hpp"
#include "kine/render/render_list.hpp"
#include "kine/render/renderer.hpp"
#include "kine/render/window.hpp"
#include "kine/resources/resource_manager.hpp"

namespace kine
{

inline bool running = false;

inline Renderer2D renderer;
inline Input global_input;

inline FlowTree* flow_tree = nullptr;

inline float delta_time() { return time::dt; }

void create(int width, int height, const char* title);
void init();
void shutdown();

void begin_frame();
void update();
void render_frame();

}  // namespace kine
