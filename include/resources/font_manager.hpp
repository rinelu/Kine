#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include "render/font.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H

namespace kine::resource
{

inline FT_Library library;
inline std::unordered_map<std::string, Font> fonts;

Font& load_font(const std::string& name, const std::string& file, int pixel_height);
Font& get_font(const std::string& name);

Font load_font_file(const std::string& name, const std::string& path);

}  // namespace kine::resource
