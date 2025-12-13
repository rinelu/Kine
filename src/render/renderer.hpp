#pragma once
#include <glad/glad.h>
#include <math.hpp>
#include <memory>
#include <vector>

#include "GLFW/glfw3.h"
#include "render_batcher.hpp"
#include "render_command.hpp"
#include "render_list.hpp"
#include "resources/resource_manager.hpp"
#include "texture2d.hpp"

namespace kine
{
struct Vertex
{
    vec2 pos;        // Final quad vertex position (pixel space)
    vec2 uv;         // Texture UV (0..1 for now)
    vec4 color;      // RGBA
    vec2 origin;     // Pivot offset (in pixels)
    vec2 size;       // Sprite size or shape size (w,h)
    vec2 misc;       // SDF params (radius, thickness, etc.)
    float rotation;  // Rotation in radians
    float type;      // 0 = sprite, 1 = rect, 2 = circle, 3 = line
};

enum class ScalingMode
{
    PixelPerfectInteger,
    FreeScale,
    LetterboxedAuto
};

class Renderer
{
   public:
    Renderer(ResourceManager* resourceManager, RenderList* _list, GLFWwindow* _window);
    ~Renderer();

    void init();
    void shutdown();

    void begin_frame();
    void end_frame();
    void render();

    // Virtual resolution system
    void set_virtual_resolution(int width, int height);
    void disable_virtual_resolution();
    void compute_virtual_scaling(int windowW, int windowH);

    // Command handlers
    void draw_sprite(const RenderCommand* cmd, const Texture2D* tex);
    void draw_rect(const RenderCommand* cmd);
    void draw_circle(const RenderCommand* cmd);
    void draw_line(const RenderCommand* cmd);

   private:
    ResourceManager* resources = nullptr;
    RenderList* list;
    std::unique_ptr<RenderBatcher> batcher;

    GLFWwindow* window = nullptr;

    bool virtual_enabled = false;

    unsigned int virtual_fbo = 0;
    unsigned int virtual_color = 0;
    unsigned int virtual_rbo = 0;

    int virtual_width = 0;
    int virtual_height = 0;

    ScalingMode scaling_mode = ScalingMode::LetterboxedAuto;
    vec2 final_scale = {1, 1};
    vec2 final_offset = {0, 0};

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    GLuint current_texture = 0;

    std::vector<Vertex> cpu_vertices;

    GLuint blit_shader = 0;

    GLuint shader = 0;

    mat4 projection;

    static constexpr size_t MAX_VERTICES = 100000;

    void create_gl_objects();
    void destroy_gl_objects();

    void begin_batch();
    void flush_batch();
    void push_quad(const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& d);

    void draw_batches();
    void draw_batches_direct();
    void draw_batches_virtual();

    void setup_projection_matrix();
    void update_vao_layout();
};

}  // namespace kine
