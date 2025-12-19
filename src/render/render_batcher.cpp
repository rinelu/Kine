#include "render_batcher.hpp"

#include "../resources/resource_manager.hpp"

#include <algorithm>

namespace kine
{

RenderBatcher::RenderBatcher(ResourceManager* resources) : resource_manager(resources)
{
    sorted.reserve(2048);
    batches.reserve(128);
}

void RenderBatcher::build(const std::vector<RenderCommand>& commands)
{
    sorted.clear();
    batches.clear();

    // Collect pointers (no copying of commands)
    for (const RenderCommand& cmd : commands) sorted.push_back(&cmd);

    // Sort deterministically: layer -> type -> texture pointer
    std::stable_sort(sorted.begin(), sorted.end(),
                     [this](const RenderCommand* a, const RenderCommand* b)
                     {
                         if (a->layer != b->layer) return a->layer < b->layer;

                         if (a->type != b->type) return a->type < b->type;

                         Texture2D* ta = nullptr;
                         Texture2D* tb = nullptr;

                         if (a->type == RenderType::Sprite)
                             ta = &resource_manager->textures().get(a->texture_name);
                         else if (a->type == RenderType::Text && a->font)
                             ta = a->font->texture;

                         if (b->type == RenderType::Sprite)
                             tb = &resource_manager->textures().get(b->texture_name);
                         else if (b->type == RenderType::Text && b->font)
                             tb = b->font->texture;

                         return ta < tb;
                     });

    RenderBatch* current = nullptr;

    for (const RenderCommand* cmd : sorted)
    {
        Texture2D* texture = nullptr;

        switch (cmd->type)
        {
        case RenderType::Sprite:
            texture = &resource_manager->textures().get(cmd->texture_name);
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
            batches.push_back(RenderBatch{.layer = cmd->layer, .type = cmd->type, .texture = texture, .commands = {}});
            current = &batches.back();
        }

        current->commands.push_back(cmd);
    }
}

const std::vector<RenderBatch>& RenderBatcher::get() const { return batches; }

}  // namespace kine
