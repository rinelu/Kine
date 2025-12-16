#include "renderer.hpp"
#include <algorithm>
#include <log.hpp>
#include <memory>
#include "GLFW/glfw3.h"
#include "glad/glad.h"

#define GL_CHECK()                                            \
    do                                                        \
    {                                                         \
        GLenum err = glGetError();                            \
        if (err != GL_NO_ERROR) LOG_ERROR("GL error: ", err); \
    } while (0)

namespace kine
{

Renderer::Renderer(ResourceManager* resourceManager, RenderList* _list, GLFWwindow* _window)
    : resources(resourceManager), render_list(_list), window(_window)
{
    batcher = std::make_unique<RenderBatcher>(resourceManager);
}

Renderer::~Renderer() { shutdown(); }

void Renderer::init()
{
    auto screen = resources->shaders().load("screen", "shaders/screen.vert", "shaders/screen.frag");
    shader = screen.program;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    create_gl_objects();

    if (virtual_enabled)
    {
        auto blit = resources->shaders().load("blit", "shaders/blit.vert", "shaders/blit.frag");
        blit_shader = blit.program;

        create_blit_objects();
        glUseProgram(blit_shader);
        glUniform1i(glGetUniformLocation(blit_shader, "uTexture"), 0);
    }

    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);
}

void Renderer::shutdown()
{
    destroy_gl_objects();
    destroy_blit_objects();

    if (virtual_enabled)
    {
        glDeleteFramebuffers(1, &virtual_fbo);
        glDeleteTextures(1, &virtual_color);
        glDeleteRenderbuffers(1, &virtual_rbo);
        virtual_enabled = false;
    }

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void Renderer::begin_frame() {}
void Renderer::end_frame() {}

void Renderer::set_virtual_resolution(int width, int height)
{
    LOG_INFO("Renderer: enabling virtual resolution ", width, "x", height);

    if (virtual_enabled)
    {
        glDeleteFramebuffers(1, &virtual_fbo);
        glDeleteTextures(1, &virtual_color);
        glDeleteRenderbuffers(1, &virtual_rbo);
    }

    if (width <= 0 || height <= 0)
    {
        LOG_ERROR("Renderer: invalid virtual resolution ", width, "x", height);
        return;
    }

    virtual_width = width;
    virtual_height = height;
    virtual_enabled = true;

    glGenFramebuffers(1, &virtual_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, virtual_fbo);
    // GL_CHECK();

    LOG_TRACE("GL context when creating FBO = ", glfwGetCurrentContext());
    // Texture
    glGenTextures(1, &virtual_color);
    glBindTexture(GL_TEXTURE_2D, virtual_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, virtual_color, 0);
    // GL_CHECK();

    // Depth/stencil
    glGenRenderbuffers(1, &virtual_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, virtual_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, virtual_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_WARN("Renderer: virtual framebuffer incomplete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::disable_virtual_resolution()
{
    if (!virtual_enabled) return;

    glDeleteFramebuffers(1, &virtual_fbo);
    glDeleteTextures(1, &virtual_color);
    glDeleteRenderbuffers(1, &virtual_rbo);
    virtual_enabled = false;
}

void Renderer::compute_virtual_scaling(int window_w, int window_h)
{
    const float target_aspect = float(virtual_width) / float(virtual_height);
    const float window_aspect = float(window_w) / float(window_h);

    switch (scaling_mode)
    {
    case ScalingMode::PixelPerfectInteger:
    {
        int scale = std::min(window_w / virtual_width, window_h / virtual_height);
        if (scale < 1) scale = 1;

        final_scale.x = final_scale.y = float(scale);
        int scaled_w = virtual_width * scale;
        int scaled_h = virtual_height * scale;

        final_offset.x = (window_w - scaled_w) / 2;
        final_offset.y = (window_h - scaled_h) / 2;
    }
    break;

    case ScalingMode::FreeScale:
        final_scale.x = float(window_w) / float(virtual_width);
        final_scale.y = float(window_h) / float(virtual_height);
        final_offset = vec2(0);
        break;

    case ScalingMode::LetterboxedAuto:
        if (window_aspect > target_aspect)
        {
            final_scale.y = float(window_h) / virtual_height;
            final_scale.x = final_scale.y;
        }
        else
        {
            final_scale.x = float(window_w) / virtual_width;
            final_scale.y = final_scale.x;
        }

        final_offset.x = (window_w - int(final_scale.x * virtual_width)) / 2;
        final_offset.y = (window_h - int(final_scale.y * virtual_height)) / 2;
        break;
    }
}

void Renderer::render()
{
    // begin_frame();

    batcher->build(render_list->get());

    if (virtual_enabled)
        draw_batches_virtual();
    else
        draw_batches_direct();

    end_frame();
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Renderer::create_gl_objects()
{
    LOG_INFO("Renderer: creating main VAO/VBO");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    // GL_CHECK();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // GL_CHECK();
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

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

void Renderer::destroy_gl_objects()
{
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

void Renderer::create_blit_objects()
{
    glGenVertexArrays(1, &blit_vao);
    glGenBuffers(1, &blit_vbo);

    glBindVertexArray(blit_vao);
    glBindBuffer(GL_ARRAY_BUFFER, blit_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(blit_vertices), blit_vertices, GL_STATIC_DRAW);

    // vec2 position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(BlitVertex), (void*) offsetof(BlitVertex, pos));
    glEnableVertexAttribArray(0);

    // vec2 UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BlitVertex), (void*) offsetof(BlitVertex, uv));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Renderer::destroy_blit_objects()
{
    if (blit_vbo)
    {
        glDeleteBuffers(1, &blit_vbo);
        blit_vbo = 0;
    }

    if (blit_vao)
    {
        glDeleteVertexArrays(1, &blit_vao);
        blit_vao = 0;
    }
}

void Renderer::setup_projection_matrix(int fbW, int fbH)
{
    if (virtual_enabled)
        projection = glm::ortho(0.0f, (float) virtual_width, (float) virtual_height, 0.0f, -1.0f, 1.0f);
    else
        projection = glm::ortho(0.0f, (float) fbW, (float) fbH, 0.0f, -1.0f, 1.0f);
}

void Renderer::push_quad(const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& d)
{
    // triangle 1
    cpu_vertices.push_back(a);
    cpu_vertices.push_back(b);
    cpu_vertices.push_back(c);

    // triangle 2
    cpu_vertices.push_back(a);
    cpu_vertices.push_back(c);
    cpu_vertices.push_back(d);
}

void Renderer::draw_batches()
{
    const auto& batches = batcher->get();
    for (const RenderBatch& batch : batches)
    {
        for (const RenderCommand* cmd : batch.commands)
        {
            switch (cmd->type)
            {
            case RenderType::Sprite:
                draw_sprite(cmd, static_cast<Texture2D*>(batch.texture));
                break;
            case RenderType::Rect:
                draw_rect(cmd);
                break;
            case RenderType::Circle:
                draw_circle(cmd);
                break;
            case RenderType::Line:
                draw_line(cmd);
                break;
            }
        }
    }
}

void Renderer::draw_batches_direct()
{
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = float(w) / float(h);
    glUniform1f(glGetUniformLocation(shader, "uAspect"), aspect);

    setup_projection_matrix(w, h);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjection"), 1, GL_FALSE, &projection[0][0]);
    draw_batches();

    flush_cpu_vertices();
}

void Renderer::draw_batches_virtual()
{
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, virtual_fbo);
    // GL_CHECK();
    glViewport(0, 0, virtual_width, virtual_height);

    glUseProgram(shader);
    float aspect = float(virtual_width) / float(virtual_height);
    glUniform1f(glGetUniformLocation(shader, "uAspect"), aspect);

    setup_projection_matrix(virtual_width, virtual_height);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjection"), 1, GL_FALSE, &projection[0][0]);

    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_batches();
    flush_cpu_vertices();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    compute_virtual_scaling(w, h);

    glViewport(0, 0, w, h);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(blit_shader);
    GLint scale_loc = glGetUniformLocation(blit_shader, "uFinalScale");
    GLint offset_loc = glGetUniformLocation(blit_shader, "uFinalOffset");
    GLint window_loc = glGetUniformLocation(blit_shader, "uWindowSize");
    GLint virtual_size = glGetUniformLocation(blit_shader, "uVirtualSize");

    glUniform2f(virtual_size, virtual_width, virtual_height);
    glUniform2f(scale_loc, final_scale.x, final_scale.y);
    glUniform2f(offset_loc, final_offset.x, final_offset.y);
    glUniform2f(window_loc, (float) w, (float) h);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, virtual_color);

    glBindVertexArray(blit_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::flush_cpu_vertices()
{
    if (cpu_vertices.empty()) return;

    if (cpu_vertices.size() > MAX_VERTICES)
    {
        LOG_ERROR("Renderer: cpu_vertices overflow: ", cpu_vertices.size());
        cpu_vertices.clear();
        return;
    }

    if (current_texture)
        glBindTexture(GL_TEXTURE_2D, current_texture);
    else
        glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(vao);
    // GL_CHECK();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cpu_vertices.size() * sizeof(Vertex), cpu_vertices.data());

    glDrawArrays(GL_TRIANGLES, 0, cpu_vertices.size());
    // GL_CHECK();

    cpu_vertices.clear();
}

void Renderer::emit_quad(const vec2& position, const vec2& size, const vec2& origin, float rotation_rad,
                         const vec4& color, float type, const vec2 uv[4])
{
    Vertex v[4]{};

    vec2 p[4] = {
        {-origin.x, -origin.y},
        {size.x - origin.x, -origin.y},
        {size.x - origin.x, size.y - origin.y},
        {-origin.x, size.y - origin.y},
    };

    mat2 rot(glm::cos(rotation_rad), -glm::sin(rotation_rad), glm::sin(rotation_rad), glm::cos(rotation_rad));

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

    push_quad(v[0], v[1], v[2], v[3]);
}

void Renderer::draw_sprite(const RenderCommand* cmd, const Texture2D* tex)
{
    if (!tex) return;

    const vec2 size(cmd->width > 0 ? cmd->width : float(tex->width),
                    cmd->height > 0 ? cmd->height : float(tex->height));

    const vec2 origin(cmd->pivotX * size.x, cmd->pivotY * size.y);
    const float rot = glm::radians(cmd->rotation);

    const vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);

    const vec2 uvs[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    emit_quad({cmd->x, cmd->y}, size, origin, rot, color,
              0.f,  // sprite
              uvs);
}

void Renderer::draw_rect(const RenderCommand* cmd)
{
    vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);
    emit_quad({cmd->x, cmd->y}, {cmd->width, cmd->height}, {cmd->pivotX * cmd->width, cmd->pivotY * cmd->height},
              glm::radians(cmd->rotation), color,
              1.f,  // rect
              nullptr);
}

void Renderer::draw_circle(const RenderCommand* cmd)
{
    vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);
    const glm::vec2 uvs[4] = {vec2{0}, {1, 0}, {1, 1}, {0, 1}};

    emit_quad({cmd->x, cmd->y}, vec2{cmd->radius * 2.f}, {cmd->radius, cmd->radius}, glm::radians(cmd->rotation), color,
              2.f,  // circle
              uvs);
}

void Renderer::draw_line(const RenderCommand* cmd)
{
    vec2 delta(cmd->x2 - cmd->x, cmd->y2 - cmd->y);
    float length = glm::length(delta);
    vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);

    float angle = std::atan2(delta.y, delta.x);

    emit_quad({cmd->x, cmd->y}, {length, cmd->radius * 2.f}, {0.f, cmd->radius}, angle, color,
              3.f,  // line
              nullptr);
}

}  // namespace kine
