#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "font_manager.hpp"
#include "log.hpp"
#include "resources/texture_manager.hpp"
#include "shader_manager.hpp"

namespace kine::resource
{

inline std::vector<std::string> search_dirs;
inline std::vector<std::string> extensions;

inline std::unordered_map<std::string, std::string> file_index;

void create();
void init();
void build();
void shutdown();

inline void add_search(std::vector<std::string> dirs, std::vector<std::string> ext)
{
    search_dirs.reserve(search_dirs.size() + dirs.size());
    search_dirs.insert(search_dirs.end(), dirs.begin(), dirs.end());

    extensions.reserve(extensions.size() + ext.size());
    extensions.insert(extensions.end(), ext.begin(), ext.end());
}

const std::string& get_path(const std::string& name);
const std::string read_file(const std::string& path);

}  // namespace kine::resource
