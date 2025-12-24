#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "GL.hpp"
#include "math.hpp"

namespace kine
{

#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8

struct ActionBinding
{
    std::vector<int> keys;
    std::vector<int> mouse_buttons;
};

class Input
{
   public:
    Input();

    void init(GLFWwindow* window);
    void begin_frame();

    void set_key_state(int key, bool down);
    void set_mouse_button_state(int button, bool down);
    void set_mouse_position(float x, float y);
    void set_mouse_scroll(float x, float y);

    bool key_down(int key) const;
    bool key_pressed(int key) const;
    bool key_released(int key) const;

    bool mouse_down(int button) const;
    bool mouse_pressed(int button) const;
    bool mouse_released(int button) const;

    void bind_key(const std::string& action, int key);
    void bind_mouse_button(const std::string& action, int button);
    bool is_action_down(const std::string& action) const;
    bool is_action_pressed(const std::string& action) const;

   private:
    std::array<bool, MAX_KEYS> current_keys;
    std::array<bool, MAX_KEYS> prev_keys;

    std::array<bool, MAX_MOUSE_BUTTONS> current_mouse;
    std::array<bool, MAX_MOUSE_BUTTONS> prev_mouse;

    vec2 mouse_position;
    vec2 prev_mouse_position;
    vec2 mouse_scroll;

    std::unordered_map<std::string, ActionBinding> bindings;
};

}  // namespace kine
