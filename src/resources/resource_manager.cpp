#include "resources/resource_manager.hpp"
#include <filesystem>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace kine
{

fs::path get_executable_dir()
{
    fs::path exe_path;

#if defined(_WIN32)
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    exe_path = buffer;

#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    _NSGetExecutablePath(buffer.data(), &size);
    exe_path = buffer;

#elif defined(__linux__)
    char buffer[4096];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    buffer[len] = '\0';
    exe_path = buffer;
#endif

    return exe_path.parent_path();
}

ResourceManager::ResourceManager()
{
    search_dirs = {"assets/"};
    extensions = {".vert", ".frag", ".glsl", ".png", ".jpg", ".ttf"};
}

void ResourceManager::init()
{
    if (search_dirs.empty() || extensions.empty())
        LOG_THROW(std::runtime_error, "ResourceManager: search directories is not specified.");

    build();

    texture_manager = new TextureManager(*this);
    shader_manager = new ShaderManager(*this);
    font_manager = new FontManager(*this);

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
    auto exe_dir = get_executable_dir();

    for (const auto& search_dir : search_dirs)
    {
        auto dir = (exe_dir / search_dir);
        fs::path root = fs::path(dir);

        if (!fs::exists(root))
        {
            LOG_WARN("ResourceManager: ", dir, " not found");
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
            LOG_TRACE("ResourceManager: Indexing ", key);
            if (file_index.contains(key)) LOG_WARN("ResourceManager: ", key, " is duplicate asset key");
            file_index[key] = path.string();
        }
    }
}

const std::string& ResourceManager::get_path(const std::string& name) const
{
    auto it = file_index.find(name);
    if (it == file_index.end()) LOG_THROW(std::runtime_error, "ResourceManager: ", name, " not indexed");

    return it->second;
}

TextureManager& ResourceManager::textures() { return *texture_manager; }
ShaderManager& ResourceManager::shaders() { return *shader_manager; }
FontManager& ResourceManager::fonts() { return *font_manager; }

}  // namespace kine
