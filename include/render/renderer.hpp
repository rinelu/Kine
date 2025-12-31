#pragma once
#include <math.hpp>
#include <memory>
#include <vector>
#include "GL.hpp"

#include "render_batcher.hpp"
#include "render_command.hpp"
#include "render_list.hpp"
#include "resources/resource_manager.hpp"
#include "texture2d.hpp"

namespace kine
{

// Per-vertex data consumed by the main 2D shader
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

/// Simple fullscreen triangle data for virtual-resolution blit
struct BlitVertex
{
    vec2 pos;
    vec2 uv;
};

enum class ScalingMode
{
    PixelPerfectInteger,  // Integer upscale only (retro)
    FreeScale,            // Stretch to window
    LetterboxedAuto       // Preserve aspect ratio
};

struct Renderer2D
{
    GLFWwindow* window = nullptr;

    // Batching
    RenderBatcher batcher;
    std::vector<Vertex> cpu_vertices;
    GLuint current_texture = 0;

    static constexpr size_t MAX_VERTICES = 100'000;

    // OpenGL objects
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint shader = 0;

    mat4 projection{1};

    // Virtual resolution
    bool virtual_enabled = false;

    int virtual_width = 0;
    int virtual_height = 0;

    GLuint virtual_fbo = 0;
    GLuint virtual_color = 0;
    GLuint virtual_rbo = 0;

    ScalingMode scaling_mode = ScalingMode::LetterboxedAuto;
    vec2 final_scale{1};
    vec2 final_offset{0};

    // Blit pass
    GLuint blit_vao = 0;
    GLuint blit_vbo = 0;
    GLuint blit_shader = 0;
};

namespace renderer2d
{
    static constexpr BlitVertex blit_vertices[6] = {
        {{-1.f, -1.f}, {0.f, 0.f}}, {{1.f, -1.f}, {1.f, 0.f}}, {{1.f, 1.f}, {1.f, 1.f}},
        {{-1.f, -1.f}, {0.f, 0.f}}, {{1.f, 1.f}, {1.f, 1.f}},  {{-1.f, 1.f}, {0.f, 1.f}},
    };

    void create(Renderer2D* r);
    void init(Renderer2D* r);
    void shutdown(Renderer2D* r);

    void begin_frame(Renderer2D* r);
    void render(Renderer2D* r);
    void end_frame(Renderer2D* r);

    // Virtual resolution system
    void set_virtual_resolution(Renderer2D* r, int width, int height);
    void disable_virtual_resolution(Renderer2D* r);
    void compute_virtual_scaling(Renderer2D* r, int windowW, int windowH);

    void draw_batches(Renderer2D* r);
    void draw_batches_direct(Renderer2D* r);
    void draw_batches_virtual(Renderer2D* r);

    void create_gl_objects(Renderer2D* r);
    void destroy_gl_objects(Renderer2D* r);

    void create_blit_objects(Renderer2D* r);
    void destroy_blit_objects(Renderer2D* r);

    void setup_projection_matrix(Renderer2D* r, int width, int height);

    void flush_cpu_vertices(Renderer2D* r);

    void set_texture(Renderer2D* r, GLuint texture);

    // Central quad generator used by all primitives
    void push_quad(Renderer2D* r, const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& d);

    void emit_quad(const vec2& position, const vec2& size, const vec2& origin, float rotation_rad, const vec4& color,
                   float type, const vec2 uv[4]);

    // Command handlers
    void draw_sprite(Renderer2D* r, const RenderCommand* cmd, const Texture2D* tex);
    void draw_rect(Renderer2D* r, const RenderCommand* cmd);
    void draw_circle(Renderer2D* r, const RenderCommand* cmd);
    void draw_line(Renderer2D* r, const RenderCommand* cmd);
    void draw_text(Renderer2D* r, const RenderCommand* cmd);
}  // namespace renderer2d
}  // namespace kine
