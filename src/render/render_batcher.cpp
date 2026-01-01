#include "kine/render/render_batcher.hpp"

#include <algorithm>
#include "kine/resources/texture_manager.hpp"

namespace kine::render_batcher
{

void create(RenderBatcher* rb)
{
    rb->sorted.reserve(2048);
    rb->batches.reserve(128);
}

void reset(RenderBatcher* rb)
{
    rb->batches.clear();
    rb->sorted.clear();
}

void build(RenderBatcher* rb, const std::vector<RenderCommand>& commands)
{
    rb->sorted.clear();
    rb->batches.clear();

    // Collect pointers (no copying of commands)
    for (const RenderCommand& cmd : commands) rb->sorted.push_back(&cmd);

    // Sort deterministically: layer -> type -> texture pointer
    std::stable_sort(rb->sorted.begin(), rb->sorted.end(),
                     [](const RenderCommand* a, const RenderCommand* b)
                     {
                         if (a->layer != b->layer) return a->layer < b->layer;

                         if (a->type != b->type) return a->type < b->type;

                         Texture2D* ta = nullptr;
                         Texture2D* tb = nullptr;

                         if (a->type == RenderType::Sprite)
                             ta = &resource::get_texture(a->texture_name);
                         else if (a->type == RenderType::Text && a->font)
                             ta = a->font->texture;

                         if (b->type == RenderType::Sprite)
                             tb = &resource::get_texture(b->texture_name);
                         else if (b->type == RenderType::Text && b->font)
                             tb = b->font->texture;

                         return ta < tb;
                     });

    RenderBatch* current = nullptr;

    for (const RenderCommand* cmd : rb->sorted)
    {
        Texture2D* texture = nullptr;

        switch (cmd->type)
        {
        case RenderType::Sprite:
            texture = &resource::get_texture(cmd->texture_name);
            break;

        case RenderType::Text:
            texture = cmd->font ? cmd->font->texture : nullptr;
            break;

        default:
            texture = nullptr;  // Rect / Line / Circle
            break;
        }

        const bool start_new_batch =
            !current || current->layer != cmd->layer || current->type != cmd->type || current->texture != texture;

        if (start_new_batch)
        {
            rb->batches.push_back(
                RenderBatch{.layer = cmd->layer, .type = cmd->type, .texture = texture, .commands = {}});
            current = &rb->batches.back();
        }

        current->commands.push_back(cmd);
    }
}

}  // namespace kine::render_batcher
