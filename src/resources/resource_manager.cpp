#include "kine/resources/resource_manager.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "kine/resources/error_texture.hpp"

#if defined(_WIN32)
#    include <windows.h>
#elif defined(__APPLE__)
#    include <mach-o/dyld.h>
#elif defined(__linux__)
#    include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace kine::resource
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

void create()
{
    search_dirs = {"assets/"};
    extensions = {".vert", ".frag", ".glsl", ".png", ".jpg", ".ttf"};
}

void init()
{
    if (search_dirs.empty() || extensions.empty()) LOG_THROW("ResourceManager: search directories is not specified.");

    build();

    LOG_INFO("ResourceManager: Indexed {} files", file_index.size());

    // error_texture = &load_texture("error", "error.png");
    error_texture = &load_embedded_texture("error", error_compressed_data, error_compressed_size);
    FT_Init_FreeType(&library);
}

void shutdown()
{
    file_index.clear();

    for (auto& [_, tex] : textures)
        if (tex.id) glDeleteTextures(1, &tex.id);

    for (auto& [_, shader] : shaders) glDeleteProgram(shader.program);

    FT_Done_FreeType(library);
}

void build()
{
    file_index.clear();
    auto exe_dir = get_executable_dir();

    for (const auto& search_dir : search_dirs)
    {
        auto dir = (exe_dir / search_dir);
        fs::path root = fs::path(dir);

        if (!fs::exists(root))
        {
            LOG_WARN("ResourceManager: {} not found", dir.string());
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
            LOG_TRACE("ResourceManager: Indexing {}", key);
            if (file_index.contains(key)) LOG_WARN("ResourceManager: {} is duplicate asset key", key);
            file_index[key] = path.string();
        }
    }
}

const std::string& get_path(const std::string& name)
{
    auto it = file_index.find(name);
    if (it == file_index.end()) LOG_THROW("ResourceManager: {} not indexed", name);

    return it->second;
}

const std::string read_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file) LOG_THROW("ResourceManager: Failed to open file {}", path);

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

}  // namespace kine::resource
