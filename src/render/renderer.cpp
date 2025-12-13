#include "renderer.hpp"
#include <algorithm>
#include <log.hpp>
#include <memory>
#include "GLFW/glfw3.h"

namespace kine
{

Renderer::Renderer(ResourceManager* resourceManager, RenderList* _list, GLFWwindow* _window)
    : resources(resourceManager), list(_list), window(_window)
{
    batcher = std::make_unique<RenderBatcher>(resourceManager);
}

Renderer::~Renderer() { shutdown(); }

void Renderer::init()
{
    auto s = resources->shaders().load("sprite", "shaders/sprite.vert", "shaders/sprite.frag");
    shader = s.program;

    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    create_gl_objects();
    setup_projection_matrix();
}

void Renderer::shutdown()
{
    destroy_gl_objects();

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
    if (virtual_enabled)
    {
        glDeleteFramebuffers(1, &virtual_fbo);
        glDeleteTextures(1, &virtual_color);
        glDeleteRenderbuffers(1, &virtual_rbo);
    }

    virtual_width = width;
    virtual_height = height;
    virtual_enabled = true;

    glGenFramebuffers(1, &virtual_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, virtual_fbo);

    // Texture
    glGenTextures(1, &virtual_color);
    glBindTexture(GL_TEXTURE_2D, virtual_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, virtual_color, 0);

    // Depth/stencil
    glGenRenderbuffers(1, &virtual_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, virtual_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, virtual_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_WARN("virtual framebuffer incomplete!");

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
    batcher->build(list->get());

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjection"), 1, GL_FALSE, &projection[0][0]);

    if (virtual_enabled)
        draw_batches_virtual();
    else
        draw_batches_direct();

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Renderer::create_gl_objects()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    // Vertex attributes layout
    GLuint attrib = 0;
    // pos
    glVertexAttribPointer(attrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glEnableVertexAttribArray(attrib++);
    // uv
    glVertexAttribPointer(attrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));
    glEnableVertexAttribArray(attrib++);
    // color
    glVertexAttribPointer(attrib, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, color));
    glEnableVertexAttribArray(attrib++);
    // origin
    glVertexAttribPointer(attrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, origin));
    glEnableVertexAttribArray(attrib++);
    // size
    glVertexAttribPointer(attrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, size));
    glEnableVertexAttribArray(attrib++);
    // misc
    glVertexAttribPointer(attrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, misc));
    glEnableVertexAttribArray(attrib++);
    // rotation
    glVertexAttribPointer(attrib, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, rotation));
    glEnableVertexAttribArray(attrib++);
    // type
    glVertexAttribPointer(attrib, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, type));
    glEnableVertexAttribArray(attrib++);

    glBindVertexArray(0);
}

void Renderer::destroy_gl_objects()
{
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

void Renderer::setup_projection_matrix()
{
    float w = float(virtual_width > 0 ? virtual_width : 800);
    float h = float(virtual_height > 0 ? virtual_height : 600);

    // Top-left origin
    projection = glm::ortho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
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
    int W, H;
    glfwGetFramebufferSize(window, &W, &H);
    glViewport(0, 0, W, H);
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_batches();

    if (!cpu_vertices.empty())
    {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, cpu_vertices.size() * sizeof(Vertex), cpu_vertices.data());

        glDrawArrays(GL_TRIANGLES, 0, cpu_vertices.size());
        cpu_vertices.clear();
    }
}

void Renderer::draw_batches_virtual()
{
    glBindFramebuffer(GL_FRAMEBUFFER, virtual_fbo);
    draw_batches_direct();

    // Blit FBO to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int W, H;
    glfwGetFramebufferSize(window, &W, &H);
    compute_virtual_scaling(W, H);
    glViewport(0, 0, W, H);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(blit_shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, virtual_color);

    // TODO: full-screen quad VAO for blit
}

void Renderer::draw_sprite(const RenderCommand* cmd, const Texture2D* tex)
{
    if (!tex) return;

    Vertex v[4]{};

    vec2 size(cmd->width > 0 ? cmd->width : float(tex->width), cmd->height > 0 ? cmd->height : float(tex->height));
    vec2 origin = vec2(cmd->pivotX * size.x, cmd->pivotY * size.y);
    vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);

    // positions relative to pivot
    vec2 p0(-origin.x, -origin.y);
    vec2 p1(size.x - origin.x, -origin.y);
    vec2 p2(size.x - origin.x, size.y - origin.y);
    vec2 p3(-origin.x, size.y - origin.y);

    // rotation
    float rad = glm::radians(cmd->rotation);
    mat2 rot = mat2(glm::cos(rad), -glm::sin(rad), glm::sin(rad), glm::cos(rad));

    p0 = rot * p0 + vec2(cmd->x, cmd->y);
    p1 = rot * p1 + vec2(cmd->x, cmd->y);
    p2 = rot * p2 + vec2(cmd->x, cmd->y);
    p3 = rot * p3 + vec2(cmd->x, cmd->y);

    v[0].pos = p0;
    v[1].pos = p1;
    v[2].pos = p2;
    v[3].pos = p3;
    v[0].uv = vec2(0, 0);
    v[1].uv = vec2(1, 0);
    v[2].uv = vec2(1, 1);
    v[3].uv = vec2(0, 1);
    v[0].color = v[1].color = v[2].color = v[3].color = color;
    v[0].origin = v[1].origin = v[2].origin = v[3].origin = origin;
    v[0].size = v[1].size = v[2].size = v[3].size = size;
    v[0].rotation = v[1].rotation = v[2].rotation = v[3].rotation = rad;
    v[0].type = v[1].type = v[2].type = v[3].type = 0.f;  // sprite

    push_quad(v[0], v[1], v[2], v[3]);
}

void Renderer::draw_rect(const RenderCommand* cmd)
{
    Vertex v[4]{};
    vec2 size(cmd->width, cmd->height);
    vec2 origin = vec2(cmd->pivotX * size.x, cmd->pivotY * size.y);
    vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);

    vec2 p0(-origin.x, -origin.y);
    vec2 p1(size.x - origin.x, -origin.y);
    vec2 p2(size.x - origin.x, size.y - origin.y);
    vec2 p3(-origin.x, size.y - origin.y);

    float rad = glm::radians(cmd->rotation);
    mat2 rot = mat2(glm::cos(rad), -glm::sin(rad), glm::sin(rad), glm::cos(rad));

    p0 = rot * p0 + vec2(cmd->x, cmd->y);
    p1 = rot * p1 + vec2(cmd->x, cmd->y);
    p2 = rot * p2 + vec2(cmd->x, cmd->y);
    p3 = rot * p3 + vec2(cmd->x, cmd->y);

    v[0].pos = p0;
    v[1].pos = p1;
    v[2].pos = p2;
    v[3].pos = p3;
    v[0].uv = v[1].uv = v[2].uv = v[3].uv = vec2(0.f);
    v[0].color = v[1].color = v[2].color = v[3].color = color;
    v[0].origin = v[1].origin = v[2].origin = v[3].origin = origin;
    v[0].size = v[1].size = v[2].size = v[3].size = size;
    v[0].rotation = v[1].rotation = v[2].rotation = v[3].rotation = rad;
    v[0].type = v[1].type = v[2].type = v[3].type = 1.f;  // rect

    push_quad(v[0], v[1], v[2], v[3]);
}

void Renderer::draw_circle(const RenderCommand* cmd)
{
    Vertex v[4]{};
    float radius = cmd->radius;
    vec2 size(radius * 2.f);
    vec2 origin(radius, radius);
    vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);

    vec2 p0(-origin.x, -origin.y);
    vec2 p1(size.x - origin.x, -origin.y);
    vec2 p2(size.x - origin.x, size.y - origin.y);
    vec2 p3(-origin.x, size.y - origin.y);

    float rad = glm::radians(cmd->rotation);
    mat2 rot = mat2(glm::cos(rad), -glm::sin(rad), glm::sin(rad), glm::cos(rad));

    p0 = rot * p0 + vec2(cmd->x, cmd->y);
    p1 = rot * p1 + vec2(cmd->x, cmd->y);
    p2 = rot * p2 + vec2(cmd->x, cmd->y);
    p3 = rot * p3 + vec2(cmd->x, cmd->y);

    v[0].pos = p0;
    v[1].pos = p1;
    v[2].pos = p2;
    v[3].pos = p3;
    v[0].uv = vec2(0, 0);
    v[1].uv = vec2(1, 0);
    v[2].uv = vec2(1, 1);
    v[3].uv = vec2(0, 1);
    v[0].color = v[1].color = v[2].color = v[3].color = color;
    v[0].origin = v[1].origin = v[2].origin = v[3].origin = origin;
    v[0].size = v[1].size = v[2].size = v[3].size = size;
    v[0].rotation = v[1].rotation = v[2].rotation = v[3].rotation = rad;
    v[0].type = v[1].type = v[2].type = v[3].type = 2.f;  // circle

    push_quad(v[0], v[1], v[2], v[3]);
}

void Renderer::draw_line(const RenderCommand* cmd)
{
    Vertex v[4]{};
    vec2 delta(cmd->x2 - cmd->x, cmd->y2 - cmd->y);
    float length = glm::length(delta);
    vec2 size(length, cmd->radius * 2.f);
    vec2 origin(0.f, cmd->radius);
    vec4 color(cmd->color[0] / 255.f, cmd->color[1] / 255.f, cmd->color[2] / 255.f, cmd->color[3] / 255.f);

    float angle = std::atan2(delta.y, delta.x);
    mat2 rot = mat2(glm::cos(angle), -glm::sin(angle), glm::sin(angle), glm::cos(angle));

    vec2 p0(0.f, -origin.y);
    vec2 p1(size.x, -origin.y);
    vec2 p2(size.x, origin.y);
    vec2 p3(0.f, origin.y);

    p0 = rot * p0 + vec2(cmd->x, cmd->y);
    p1 = rot * p1 + vec2(cmd->x, cmd->y);
    p2 = rot * p2 + vec2(cmd->x, cmd->y);
    p3 = rot * p3 + vec2(cmd->x, cmd->y);

    v[0].pos = p0;
    v[1].pos = p1;
    v[2].pos = p2;
    v[3].pos = p3;
    v[0].uv = v[1].uv = v[2].uv = v[3].uv = vec2(0.f);
    v[0].color = v[1].color = v[2].color = v[3].color = color;
    v[0].origin = v[1].origin = v[2].origin = v[3].origin = origin;
    v[0].size = v[1].size = v[2].size = v[3].size = size;
    v[0].rotation = v[1].rotation = v[2].rotation = v[3].rotation = 0.f;
    v[0].type = v[1].type = v[2].type = v[3].type = 3.f;  // line

    push_quad(v[0], v[1], v[2], v[3]);
}

}  // namespace kine
