#pragma once
#include <vector>
#include "render_command.hpp"

namespace kine
{

struct RenderBatch
{
    int32_t layer;
    RenderType type;
    Texture2D* texture;
    std::vector<const RenderCommand*> commands;
};

struct RenderBatcher
{
    // Temporary sorted command list (pointers only).
    std::vector<const RenderCommand*> sorted;
    std::vector<RenderBatch> batches;
};

namespace render_batcher
{
    void create(RenderBatcher* rb);
    void reset(RenderBatcher* rb);

    // Builds sorted and batched render groups from raw commands.
    void build(RenderBatcher* rb, const std::vector<RenderCommand>& commands);
}  // namespace render_batcher
}  // namespace kine
