#include "resource_manager.hpp"
#include <filesystem>
#include <log.hpp>
#include <stdexcept>

namespace fs = std::filesystem;

namespace kine
{

ResourceManager::ResourceManager()
{
    search_dirs = {"assets/"};
    extensions = {".vert", ".frag", ".glsl"};
}

void ResourceManager::init()
{
    build();

    shader_manager = new ShaderManager(*this);

    LOG_INFO("ResourceManager: Indexed ", file_index.size(), " files");
}

void ResourceManager::shutdown()
{
    delete shader_manager;
    shader_manager = nullptr;

    file_index.clear();
}

void ResourceManager::build()
{
    file_index.clear();

    for (const auto& dir : search_dirs)
    {
        fs::path root = fs::path(dir);

        if (!fs::exists(root))
        {
            LOG_WARN("ResourceManager: Directory not found: ", dir);
            continue;
        }

        for (const auto& entry : fs::recursive_directory_iterator(root))
        {
            if (!entry.is_regular_file()) continue;

            fs::path path = entry.path();
            std::string ext = path.extension().string();

            if (std::find(extensions.begin(), extensions.end(), ext) == extensions.end()) continue;

            fs::path relative = fs::relative(path, root);
            // relative.replace_extension("");

            std::string key = relative.generic_string();
            LOG_INFO("ResourceManager: Indexing : ", key);
            if (file_index.contains(key)) LOG_WARN("ResourceManager: Duplicate asset key: ", key);
            file_index[key] = path.string();
        }
    }
}

const std::string& ResourceManager::get_path(const std::string& name) const
{
    auto it = file_index.find(name);
    if (it == file_index.end()) LOG_THROW(std::runtime_error, "ResourceManager: Resource not indexed: ", name);

    return it->second;
}

ShaderManager& ResourceManager::shaders() { return *shader_manager; }

}  // namespace kine
