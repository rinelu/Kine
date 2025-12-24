#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include "render/font.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H

namespace kine
{

class ResourceManager;

class FontManager
{
   public:
    explicit FontManager(ResourceManager& rm);
    ~FontManager();

    Font& load(const std::string& name, const std::string& file, int pixel_height);
    Font& get(const std::string& name);

   private:
    Font load_from_file(const std::string& name, const std::string& path);

    FT_Library library;
    ResourceManager& resources;
    std::unordered_map<std::string, Font> fonts;
};
}  // namespace kine
