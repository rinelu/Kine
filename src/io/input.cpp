#include "io/input.hpp"

// TODO REFACTOR
namespace kine
{

namespace input
{
    static Input* instance = nullptr;

    static void key_callback(GLFWwindow*, int key, int, int action, int)
    {
        if (!instance || key < 0 || key >= MAX_KEYS) return;

        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            instance->set_key_state(key, true);
        else if (action == GLFW_RELEASE)
            instance->set_key_state(key, false);
    }

    static void mouse_button_callback(GLFWwindow*, int button, int action, int)
    {
        if (!instance || button < 0 || button >= MAX_MOUSE_BUTTONS) return;

        if (action == GLFW_PRESS)
            instance->set_mouse_button_state(button, true);
        else if (action == GLFW_RELEASE)
            instance->set_mouse_button_state(button, false);
    }

    static void cursor_position_callback(GLFWwindow*, double x, double y)
    {
        if (!instance) return;
        instance->set_mouse_position((float) x, (float) y);
    }

    static void scroll_callback(GLFWwindow*, double xOffset, double yOffset)
    {
        if (!instance) return;
        instance->set_mouse_scroll((float) xOffset, (float) yOffset);
    }
}  // namespace input

Input::Input()
{
    current_keys.fill(false);
    prev_keys.fill(false);
    current_mouse.fill(false);
    prev_mouse.fill(false);

    mouse_position = {0.0f, 0.0f};
    prev_mouse_position = mouse_position;
    mouse_scroll = {0.0f, 0.0f};

    input::instance = this;
}

void Input::init(GLFWwindow* window)
{
    glfwSetKeyCallback(window, input::key_callback);
    glfwSetMouseButtonCallback(window, input::mouse_button_callback);
    glfwSetCursorPosCallback(window, input::cursor_position_callback);
    glfwSetScrollCallback(window, input::scroll_callback);
}

void Input::begin_frame()
{
    prev_keys = current_keys;
    prev_mouse = current_mouse;
    prev_mouse_position = mouse_position;

    mouse_scroll = {0.0f, 0.0f};  // reset every frame
}

void Input::set_key_state(int key, bool down) { current_keys[key] = down; }
void Input::set_mouse_button_state(int button, bool down) { current_mouse[button] = down; }
void Input::set_mouse_position(float x, float y) { mouse_position = {x, y}; }
void Input::set_mouse_scroll(float x, float y) { mouse_scroll = {x, y}; }

bool Input::key_down(int key) const
{
    if (key < 0 || key >= MAX_KEYS) return false;
    return current_keys[key];
}
bool Input::key_pressed(int key) const
{
    if (key < 0 || key >= MAX_KEYS) return false;
    return current_keys[key] && !prev_keys[key];
}
bool Input::key_released(int key) const
{
    if (key < 0 || key >= MAX_KEYS) return false;
    return !current_keys[key] && prev_keys[key];
}
bool Input::mouse_down(int button) const
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return false;
    return current_mouse[button];
}
bool Input::mouse_pressed(int button) const
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return false;
    return current_mouse[button] && !prev_mouse[button];
}
bool Input::mouse_released(int button) const
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return false;
    return !current_mouse[button] && prev_mouse[button];
}
void Input::bind_key(const std::string& action, int key) { bindings[action].keys.push_back(key); }
void Input::bind_mouse_button(const std::string& action, int button)
{
    bindings[action].mouse_buttons.push_back(button);
}

bool Input::is_action_down(const std::string& action) const
{
    auto it = bindings.find(action);
    if (it == bindings.end()) return false;

    for (int key : it->second.keys)
        if (key_down(key)) return true;

    for (int button : it->second.mouse_buttons)
        if (mouse_down(button)) return true;

    return false;
}

bool Input::is_action_pressed(const std::string& action) const
{
    auto it = bindings.find(action);
    if (it == bindings.end()) return false;

    for (int key : it->second.keys)
        if (key_pressed(key)) return true;

    for (int button : it->second.mouse_buttons)
        if (mouse_pressed(button)) return true;

    return false;
}

}  // namespace kine
