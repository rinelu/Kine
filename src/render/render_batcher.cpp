#include "render_batcher.hpp"
#include "../resources/resource_manager.hpp"
#include "log.hpp"

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

    // Point sorted to existing commands
    for (const auto& cmd : commands) sorted.push_back(&cmd);

    // Deterministic stable sort by layer
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const RenderCommand* a, const RenderCommand* b) { return a->layer < b->layer; });

    // Group by texture name
    // Hash map ensures one batch per texture
    std::unordered_map<void*, size_t> batch_map;
    batch_map.reserve(64);

    for (const RenderCommand* cmd : sorted)
    {
        void* texture_ptr = nullptr;

        if (cmd->type == RenderType::Sprite)
        {
            // Resolve texture name to Texture2D*
            // ResourceManager returns Texture2D&,
            // auto& texture = resource_manager->get_texture(cmd->texture_name);
            // texture_ptr = static_cast<void*>(&texture);
        }

        // Create new batch if necessary
        if (!batch_map.contains(texture_ptr))
        {
            const size_t size = batches.size();
            batch_map[texture_ptr] = size;

            RenderBatch newBatch;
            newBatch.texture = texture_ptr;
            batches.push_back(newBatch);
        }

        size_t batch = batch_map[texture_ptr];
        batches[batch].commands.push_back(cmd);
    }
}

const std::vector<RenderBatch>& RenderBatcher::get() const { return batches; }

}  // namespace kine
