#include "kine/io/input.hpp"

#include "GLFW/glfw3.h"

namespace kine::input
{

#define GET_INPUT                                                     \
    Input* i = static_cast<Input*>(glfwGetWindowUserPointer(window)); \
    if (!i) return;

static void key_callback(GLFWwindow* window, int key, int, int action, int)
{
    GET_INPUT;
    if (key < 0 || key >= MAX_KEYS) return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
        input::set_key_state(i, key, true);
    else if (action == GLFW_RELEASE)
        input::set_key_state(i, key, false);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int)
{
    GET_INPUT;
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return;

    if (action == GLFW_PRESS)
        input::set_mouse_button_state(i, button, true);
    else if (action == GLFW_RELEASE)
        input::set_mouse_button_state(i, button, false);
}

static void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    GET_INPUT;
    input::set_mouse_position(i, (float) x, (float) y);
}

static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    GET_INPUT;
    input::set_mouse_scroll(i, (float) xOffset, (float) yOffset);
}
#undef GET_INPUT

void create(Input* i)
{
    i->current_keys.fill(false);
    i->prev_keys.fill(false);
    i->current_mouse.fill(false);
    i->prev_mouse.fill(false);

    i->mouse_position = {0.0f, 0.0f};
    i->prev_mouse_position = i->mouse_position;
    i->mouse_scroll = {0.0f, 0.0f};
}

void init(Input* i)
{
    glfwSetWindowUserPointer(window::get(), i);
    glfwSetKeyCallback(window::get(), input::key_callback);
    glfwSetMouseButtonCallback(window::get(), input::mouse_button_callback);
    glfwSetCursorPosCallback(window::get(), input::cursor_position_callback);
    glfwSetScrollCallback(window::get(), input::scroll_callback);
}

void begin_frame(Input* i)
{
    i->prev_keys = i->current_keys;
    i->prev_mouse = i->current_mouse;
    i->prev_mouse_position = i->mouse_position;

    i->mouse_scroll = {0.0f, 0.0f};  // reset every frame
}

void shutdown(Input* i)
{
    i->current_keys = {};
    i->prev_keys = {};
    i->current_mouse = {};
    i->prev_mouse = {};

    i->mouse_position = {};
    i->prev_mouse_position = {};
    i->mouse_scroll = {};

    glfwSetWindowUserPointer(window::get(), NULL);
}

void set_key_state(Input* i, int key, bool down) { i->current_keys[key] = down; }
void set_mouse_button_state(Input* i, int button, bool down) { i->current_mouse[button] = down; }
void set_mouse_position(Input* i, float x, float y) { i->mouse_position = {x, y}; }
void set_mouse_scroll(Input* i, float x, float y) { i->mouse_scroll = {x, y}; }

#define VALID key < 0 || key >= MAX_KEYS
bool key_down(Input* i, int key)
{
    if (VALID) return false;
    return i->current_keys[key];
}
bool key_pressed(Input* i, int key)
{
    if (VALID) return false;
    return i->current_keys[key] && !i->prev_keys[key];
}
bool key_released(Input* i, int key)
{
    if (VALID) return false;
    return !i->current_keys[key] && i->prev_keys[key];
}
#undef VALID
#define VALID button < 0 || button >= MAX_MOUSE_BUTTONS
bool mouse_down(Input* i, int button)
{
    if (VALID) return false;
    return i->current_mouse[button];
}
bool mouse_pressed(Input* i, int button)
{
    if (VALID) return false;
    return i->current_mouse[button] && !i->prev_mouse[button];
}
bool mouse_released(Input* i, int button)
{
    if (VALID) return false;
    return !i->current_mouse[button] && i->prev_mouse[button];
}
#undef VALID

void bind_key(Input* i, const std::string& action, int key) { i->bindings[action].keys.push_back(key); }
void bind_mouse_button(Input* i, const std::string& action, int button)
{
    i->bindings[action].mouse_buttons.push_back(button);
}

bool is_action_down(Input* i, const std::string& action)
{
    auto it = i->bindings.find(action);
    if (it == i->bindings.end()) return false;

    for (int key : it->second.keys)
        if (key_down(i, key)) return true;

    for (int button : it->second.mouse_buttons)
        if (mouse_down(i, button)) return true;

    return false;
}

bool is_action_pressed(Input* i, const std::string& action)
{
    auto it = i->bindings.find(action);
    if (it == i->bindings.end()) return false;

    for (int key : it->second.keys)
        if (key_pressed(i, key)) return true;

    for (int button : it->second.mouse_buttons)
        if (mouse_pressed(i, button)) return true;

    return false;
}

}  // namespace kine::input
