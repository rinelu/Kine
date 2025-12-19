#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include "render/texture2d.hpp"

namespace kine
{

class ResourceManager;

class TextureManager
{
   public:
    explicit TextureManager(ResourceManager& rm);
    ~TextureManager();

    Texture2D& load(const std::string& name, const std::string& file);
    Texture2D& get(const std::string& name);
    Texture2D& add(const std::string& name, Texture2D&& tex);

   private:
    Texture2D* error_texture = nullptr;

    Texture2D load_from_file(const std::string& name, const std::string& path);

    ResourceManager& resources;
    std::unordered_map<std::string, Texture2D> textures;

};

}  // namespace kine
