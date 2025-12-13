#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "shader_manager.hpp"

namespace kine
{

class ResourceManager
{
public:
    ResourceManager();

    void init();
    void shutdown();

    ShaderManager& shaders();

    const std::string& get_path(const std::string& name) const;

private:
    void build();

    std::vector<std::string> search_dirs;
    std::vector<std::string> extensions;

    std::unordered_map<std::string, std::string> file_index;

    ShaderManager* shader_manager = nullptr;
};

} // namespace kine
