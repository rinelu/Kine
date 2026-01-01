#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include "render/texture2d.hpp"

namespace kine::resource
{

inline Texture2D* error_texture = nullptr;
inline std::unordered_map<std::string, Texture2D> textures;

Texture2D& load_texture(const std::string& name, const std::string& file);
Texture2D& get_texture(const std::string& name);
Texture2D& add_texture(const std::string& name, Texture2D&& tex);
Texture2D& load_embedded_texture(const std::string& name, const unsigned char* buffer, const unsigned int len);

Texture2D load_texture_file(const std::string& name, const std::string& path);

}  // namespace kine::resource
