#pragma once
#include <vector>
#include "render_command.hpp"

namespace kine
{

class ResourceManager;

struct RenderBatch
{
    int32_t layer;
    RenderType type;
    Texture2D* texture;
    std::vector<const RenderCommand*> commands;
};
;

// Groups render commands into texture-based batches.
class RenderBatcher
{
   public:
    RenderBatcher(ResourceManager* resources);

    // Builds sorted and batched render groups from raw commands.
    void build(const std::vector<RenderCommand>& commands);

    // Provides read-only access to the generated batches.
    const std::vector<RenderBatch>& get() const;

   private:
    ResourceManager* resource_manager{nullptr};

    // Temporary sorted command list (pointers only).
    std::vector<const RenderCommand*> sorted;

    std::vector<RenderBatch> batches;
};

}  // namespace sf
