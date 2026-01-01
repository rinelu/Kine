#include "render/renderer.hpp"
#include <algorithm>
#include <memory>
#include "render/renderer2d_shaders.hpp"
#include "render/window.hpp"
#include "resources/shader_manager.hpp"

#define GL_CHECK()                                              \
    do                                                          \
    {                                                           \
        GLenum err = glGetError();                              \
        if (err != GL_NO_ERROR) LOG_ERROR("GL error: {}", err); \
    } while (0)

namespace kine::renderer2d
{

void create(Renderer2D* r)
{
    r->window = window::get();
    render_batcher::create(&r->batcher);
}

void init(Renderer2D* r)
{
    r->shader = resource::load_shader_str(screen_vert, screen_frag);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    create_gl_objects(r);

    if (r->virtual_enabled)
    {
        r->blit_shader = resource::load_shader_str(blit_vert, blit_frag);

        create_blit_objects(r);
        glUseProgram(r->blit_shader);
        glUniform1i(glGetUniformLocation(r->blit_shader, "uTexture"), 0);
    }

    glUseProgram(r->shader);
    glUniform1i(glGetUniformLocation(r->shader, "uTexture"), 0);
}

void shutdown(Renderer2D* r)
{
    render_batcher::reset(&r->batcher);

    destroy_gl_objects(r);
    destroy_blit_objects(r);

    if (r->virtual_enabled)
    {
        glDeleteFramebuffers(1, &r->virtual_fbo);
        glDeleteTextures(1, &r->virtual_color);
        glDeleteRenderbuffers(1, &r->virtual_rbo);
        r->virtual_enabled = false;
    }

    // TODO: Move to window.hpp
    if (r->window)
    {
        glfwDestroyWindow(r->window);
        r->window = nullptr;
    }
    glfwTerminate();
}

void begin_frame(Renderer2D*) {}
void render(Renderer2D* r)
{
    begin_frame(r);

    render_batcher::build(&r->batcher, render::get());

    if (r->virtual_enabled)
        draw_batches_virtual(r);
    else
        draw_batches_direct(r);

    end_frame(r);
    glfwSwapBuffers(r->window);
    render::clear();
}
void end_frame(Renderer2D*) {}

void set_virtual_resolution(Renderer2D* r, int width, int height)
{
    LOG_INFO("Renderer: enabling virtual resolution  {}x{}", width, height);

    if (r->virtual_enabled)
    {
        glDeleteFramebuffers(1, &r->virtual_fbo);
        glDeleteTextures(1, &r->virtual_color);
        glDeleteRenderbuffers(1, &r->virtual_rbo);
    }

    if (width <= 0 || height <= 0)
    {
        LOG_ERROR("Renderer: invalid virtual resolution  {}x{}", width, height);
        return;
    }

    r->virtual_width = width;
    r->virtual_height = height;
    r->virtual_enabled = true;

    glGenFramebuffers(1, &r->virtual_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, r->virtual_fbo);
    // GL_CHECK();

    // Texture
    glGenTextures(1, &r->virtual_color);
    glBindTexture(GL_TEXTURE_2D, r->virtual_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r->virtual_color, 0);
    // GL_CHECK();

    // Depth/stencil
    glGenRenderbuffers(1, &r->virtual_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, r->virtual_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, r->virtual_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_WARN("Renderer: virtual framebuffer incomplete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void disable_virtual_resolution(Renderer2D* r)
{
    if (!r->virtual_enabled) return;

    glDeleteFramebuffers(1, &r->virtual_fbo);
    glDeleteTextures(1, &r->virtual_color);
    glDeleteRenderbuffers(1, &r->virtual_rbo);
    r->virtual_enabled = false;
}

void compute_virtual_scaling(Renderer2D* r, int window_w, int window_h)
{
    const float target_aspect = float(r->virtual_width) / float(r->virtual_height);
    const float window_aspect = float(window_w) / float(window_h);

    switch (r->scaling_mode)
    {
    case ScalingMode::PixelPerfectInteger:
    {
        int scale = std::min(window_w / r->virtual_width, window_h / r->virtual_height);
        if (scale < 1) scale = 1;

        r->final_scale.x = r->final_scale.y = float(scale);
        int scaled_w = r->virtual_width * scale;
        int scaled_h = r->virtual_height * scale;

        r->final_offset.x = (window_w - scaled_w) / 2;
        r->final_offset.y = (window_h - scaled_h) / 2;
    }
    break;

    case ScalingMode::FreeScale:
        r->final_scale.x = float(window_w) / float(r->virtual_width);
        r->final_scale.y = float(window_h) / float(r->virtual_height);
        r->final_offset = vec2(0);
        break;

    case ScalingMode::LetterboxedAuto:
        if (window_aspect > target_aspect)
        {
            r->final_scale.y = float(window_h) / r->virtual_height;
            r->final_scale.x = r->final_scale.y;
        }
        else
        {
            r->final_scale.x = float(window_w) / r->virtual_width;
            r->final_scale.y = r->final_scale.x;
        }

        r->final_offset.x = (window_w - int(r->final_scale.x * r->virtual_width)) / 2;
        r->final_offset.y = (window_h - int(r->final_scale.y * r->virtual_height)) / 2;
        break;
    }
}

void draw_batches(Renderer2D* r)
{
    const auto& batches = r->batcher.batches;
    for (const RenderBatch& batch : batches)
    {
        for (const RenderCommand* cmd : batch.commands)
        {
            switch (cmd->type)
            {
            case RenderType::Sprite:
                draw_sprite(r, cmd, static_cast<Texture2D*>(batch.texture));
                break;
            case RenderType::Rect:
                draw_rect(r, cmd);
                break;
            case RenderType::Circle:
                draw_circle(r, cmd);
                break;
            case RenderType::Line:
                draw_line(r, cmd);
                break;
            case RenderType::Text:
                draw_text(r, cmd);
                break;
            }
        }
    }
}

void draw_batches_direct(Renderer2D* r)
{
    int w, h;
    glfwGetFramebufferSize(r->window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // glUseProgram(shader);
    float aspect = float(w) / float(h);
    glUniform1f(glGetUniformLocation(r->shader, "uAspect"), aspect);

    setup_projection_matrix(r, w, h);
    glUniformMatrix4fv(glGetUniformLocation(r->shader, "uProjection"), 1, GL_FALSE, &r->projection[0][0]);
    draw_batches(r);
    flush_cpu_vertices(r);
}

void draw_batches_virtual(Renderer2D* r)
{
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, r->virtual_fbo);
    // GL_CHECK();
    glViewport(0, 0, r->virtual_width, r->virtual_height);
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(r->shader);
    glBindVertexArray(r->vao);
    float aspect = float(r->virtual_width) / float(r->virtual_height);
    glUniform1f(glGetUniformLocation(r->shader, "uAspect"), aspect);

    setup_projection_matrix(r, r->virtual_width, r->virtual_height);
    glUniformMatrix4fv(glGetUniformLocation(r->shader, "uProjection"), 1, GL_FALSE, &r->projection[0][0]);
    draw_batches(r);
    flush_cpu_vertices(r);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int w, h;
    glfwGetFramebufferSize(r->window, &w, &h);
    compute_virtual_scaling(r, w, h);

    glViewport(0, 0, w, h);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(r->blit_shader);
    GLint scale_loc = glGetUniformLocation(r->blit_shader, "uFinalScale");
    GLint offset_loc = glGetUniformLocation(r->blit_shader, "uFinalOffset");
    GLint window_loc = glGetUniformLocation(r->blit_shader, "uWindowSize");
    GLint virtual_size = glGetUniformLocation(r->blit_shader, "uVirtualSize");

    glUniform2f(virtual_size, r->virtual_width, r->virtual_height);
    glUniform2f(scale_loc, r->final_scale.x, r->final_scale.y);
    glUniform2f(offset_loc, r->final_offset.x, r->final_offset.y);
    glUniform2f(window_loc, (float) w, (float) h);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->virtual_color);

    glBindVertexArray(r->blit_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    set_texture(r, 0);
    glEnable(GL_DEPTH_TEST);
}

void create_gl_objects(Renderer2D* r)
{
    LOG_DEBUG("Renderer: creating main VAO/VBO");

    glGenVertexArrays(1, &r->vao);
    glGenBuffers(1, &r->vbo);

    glBindVertexArray(r->vao);
    // GL_CHECK();
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    // GL_CHECK();
    glBufferData(GL_ARRAY_BUFFER, r->MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    GLuint attrib = 0;
#define ATTR(count, type, member)                                                                           \
    glVertexAttribPointer(attrib, count, type, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, member)); \
    glEnableVertexAttribArray(attrib++)

    ATTR(2, GL_FLOAT, pos);
    ATTR(2, GL_FLOAT, uv);
    ATTR(4, GL_FLOAT, color);
    ATTR(2, GL_FLOAT, origin);
    ATTR(2, GL_FLOAT, size);
    ATTR(2, GL_FLOAT, misc);
    ATTR(1, GL_FLOAT, rotation);
    ATTR(1, GL_FLOAT, type);

#undef ATTR

    glBindVertexArray(0);
}

void destroy_gl_objects(Renderer2D* r)
{
    if (r->vbo) glDeleteBuffers(1, &r->vbo);
    if (r->vao) glDeleteVertexArrays(1, &r->vao);
}

void create_blit_objects(Renderer2D* r)
{
    glGenVertexArrays(1, &r->blit_vao);
    glGenBuffers(1, &r->blit_vbo);

    glBindVertexArray(r->blit_vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->blit_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(blit_vertices), blit_vertices, GL_STATIC_DRAW);

    // vec2 position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(BlitVertex), (void*) offsetof(BlitVertex, pos));
    glEnableVertexAttribArray(0);

    // vec2 UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BlitVertex), (void*) offsetof(BlitVertex, uv));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void destroy_blit_objects(Renderer2D* r)
{
    if (r->blit_vbo)
    {
        glDeleteBuffers(1, &r->blit_vbo);
        r->blit_vbo = 0;
    }

    if (r->blit_vao)
    {
        glDeleteVertexArrays(1, &r->blit_vao);
        r->blit_vao = 0;
    }
}

void setup_projection_matrix(Renderer2D* r, int width, int height)
{
    if (r->virtual_enabled)
        r->projection = glm::ortho(0.0f, (float) r->virtual_width, (float) r->virtual_height, 0.0f, -1.0f, 1.0f);
    else
        r->projection = glm::ortho(0.0f, (float) width, (float) height, 0.0f, -1.0f, 1.0f);
}

void push_quad(Renderer2D* r, const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& d)
{
    // triangle 1
    r->cpu_vertices.push_back(a);
    r->cpu_vertices.push_back(b);
    r->cpu_vertices.push_back(c);

    // triangle 2
    r->cpu_vertices.push_back(a);
    r->cpu_vertices.push_back(c);
    r->cpu_vertices.push_back(d);
}

void flush_cpu_vertices(Renderer2D* r)
{
    if (r->cpu_vertices.empty()) return;

    if (r->cpu_vertices.size() > r->MAX_VERTICES)
    {
        LOG_ERROR("Renderer: cpu_vertices overflow {}", r->cpu_vertices.size());
        r->cpu_vertices.clear();
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->current_texture ? r->current_texture : 0);
    glBindVertexArray(r->vao);
    // GL_CHECK();
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->cpu_vertices.size() * sizeof(Vertex), r->cpu_vertices.data());

    glDrawArrays(GL_TRIANGLES, 0, r->cpu_vertices.size());
    // GL_CHECK();

    r->cpu_vertices.clear();
}

void set_texture(Renderer2D* r, GLuint texture)
{
    if (r->current_texture == texture) return;
    flush_cpu_vertices(r);
    r->current_texture = texture;
}

void emit_quad(Renderer2D* r, const vec2& position, const vec2& size, const vec2& origin, float rotation_rad,
               const vec4& color, float type, const vec2 uv[4])
{
    Vertex v[4]{};

    vec2 p[4] = {
        {-origin.x, -origin.y},
        {size.x - origin.x, -origin.y},
        {size.x - origin.x, size.y - origin.y},
        {-origin.x, size.y - origin.y},
    };

    mat2 rot(glm::cos(rotation_rad), glm::sin(rotation_rad), -glm::sin(rotation_rad), glm::cos(rotation_rad));

    for (int i = 0; i < 4; ++i)
    {
        v[i].pos = rot * p[i] + position;
        v[i].uv = uv ? uv[i] : vec2(0);
        v[i].color = color;
        v[i].origin = origin;
        v[i].size = size;
        v[i].rotation = rotation_rad;
        v[i].type = type;
    }

    push_quad(r, v[0], v[1], v[2], v[3]);
}

void draw_sprite(Renderer2D* r, const RenderCommand* cmd, const Texture2D* tex)
{
    if (!tex) return;
    set_texture(r, tex->id);

    const vec2 size(float(tex->width), float(tex->height));

    const vec2 origin(cmd->pivotX * size.x, cmd->pivotY * size.y);
    const float rot = glm::radians(cmd->rotation);
    const vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);
    const vec2 uvs[4] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};

    emit_quad(r, {cmd->x, cmd->y}, size, origin, rot, color,
              0.f,  // sprite
              uvs);
}

void draw_rect(Renderer2D* r, const RenderCommand* cmd)
{
    vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);
    emit_quad(r, {cmd->x, cmd->y}, {cmd->width, cmd->height}, {cmd->pivotX * cmd->width, cmd->pivotY * cmd->height},
              glm::radians(cmd->rotation), color,
              1.f,  // rect
              nullptr);
}

void draw_circle(Renderer2D* r, const RenderCommand* cmd)
{
    vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);
    const glm::vec2 uvs[4] = {vec2{0}, {1, 0}, {1, 1}, {0, 1}};

    emit_quad(r, {cmd->x, cmd->y}, vec2{cmd->radius * 2.f}, {cmd->radius, cmd->radius}, glm::radians(cmd->rotation),
              color,
              2.f,  // circle
              uvs);
}

void draw_line(Renderer2D* r, const RenderCommand* cmd)
{
    vec2 delta(cmd->x2 - cmd->x, cmd->y2 - cmd->y);
    float length = glm::length(delta);
    vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);

    float angle = std::atan2(delta.y, delta.x);

    emit_quad(r, {cmd->x, cmd->y}, {length, cmd->radius * 2.f}, {0.f, cmd->radius}, angle, color,
              3.f,  // line
              nullptr);
}

void draw_text(Renderer2D* r, const RenderCommand* cmd)
{
    if (!cmd || !cmd->font || cmd->text.empty()) return;

    const Font& font = *cmd->font;
    set_texture(r, font.texture->id);

    const vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);
    const float rotation = glm::radians(cmd->rotation);
    const float scale = (cmd->scale != 0.f) ? cmd->scale : 1.f;

    // Measure text block for pivot
    float line_width = 0.f;
    float max_width = 0.f;
    float total_height = font.line_height * scale;

    for (char c : cmd->text)
    {
        if (c == '\n')
        {
            max_width = std::max(max_width, line_width);
            line_width = 0.f;
            total_height += font.line_height * scale;
            continue;
        }

        auto it = font.glyphs.find(c);
        if (it == font.glyphs.end()) continue;
        line_width += it->second.advance * scale;
    }

    max_width = std::max(max_width, line_width);
    vec2 origin(cmd->pivotX * max_width, cmd->pivotY * total_height);

    // Emit glyph quads
    vec2 pen(0.f);
    for (char c : cmd->text)
    {
        if (c == '\n')
        {
            pen.x = 0.f;
            pen.y += font.line_height * scale;
            continue;
        }

        auto it = font.glyphs.find(c);
        if (it == font.glyphs.end()) continue;

        const Glyph& g = it->second;
        float baseline = cmd->y + pen.y + font.ascent * scale;
        const vec2 glyph_pos(cmd->x + pen.x + g.bearing.x * scale, baseline - g.bearing.y * scale);
        const vec2 glyph_size(g.size.x * scale, g.size.y * scale);

        emit_quad(r, glyph_pos, glyph_size, origin, rotation, color,
                  4.f,  // text
                  g.uv);
        pen.x += g.advance * scale;
    }
}

}  // namespace kine::renderer2d

#undef GL_CHECK
