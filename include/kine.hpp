#pragma once

#include "core/scheduler.hpp"
#include "core/time.hpp"
#include "flow/flow_tree.hpp"
#include "io/input.hpp"
#include "render/render_list.hpp"
#include "render/renderer.hpp"
#include "render/window.hpp"
#include "resources/resource_manager.hpp"

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
