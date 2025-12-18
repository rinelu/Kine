#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "resources/texture_manager.hpp"
#include "shader_manager.hpp"

namespace kine
{

class ResourceManager
{
public:
    ResourceManager();

    void init();
    void shutdown();

    TextureManager& textures();
    ShaderManager& shaders();

    const std::string& get_path(const std::string& name) const;

private:
    void build();

    std::vector<std::string> search_dirs;
    std::vector<std::string> extensions;

    std::unordered_map<std::string, std::string> file_index;

    TextureManager* texture_manager = nullptr;
    ShaderManager* shader_manager = nullptr;
};

} // namespace kine
