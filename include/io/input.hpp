#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "math.hpp"
#include "render/window.hpp"

namespace kine
{

#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8

struct ActionBinding
{
    std::vector<int> keys;
    std::vector<int> mouse_buttons;
};

struct Input
{
    std::array<bool, MAX_KEYS> current_keys;
    std::array<bool, MAX_KEYS> prev_keys;

    std::array<bool, MAX_MOUSE_BUTTONS> current_mouse;
    std::array<bool, MAX_MOUSE_BUTTONS> prev_mouse;

    vec2 mouse_position;
    vec2 prev_mouse_position;
    vec2 mouse_scroll;

    std::unordered_map<std::string, ActionBinding> bindings;
};

namespace input
{
    void create(Input* i);

    void init(Input* i);
    void begin_frame(Input* i);
    void shutdown(Input* i);

    void set_key_state(Input* i, int key, bool down);
    void set_mouse_button_state(Input* i, int button, bool down);
    void set_mouse_position(Input* i, float x, float y);
    void set_mouse_scroll(Input* i, float x, float y);

    bool key_down(Input* i, int key);
    bool key_pressed(Input* i, int key);
    bool key_released(Input* i, int key);

    bool mouse_down(Input* i, int button);
    bool mouse_pressed(Input* i, int button);
    bool mouse_released(Input* i, int button);

    void bind_key(Input* i, const std::string& action, int key);
    void bind_mouse_button(Input* i, const std::string& action, int button);
    bool is_action_down(Input* i, const std::string& action);
    bool is_action_pressed(Input* i, const std::string& action);
}  // namespace input

}  // namespace kine
