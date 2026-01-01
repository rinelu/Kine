#include <algorithm>
#include "kine/resources/resource_manager.hpp"
#include "kine/resources/texture_manager.hpp"

namespace kine::resource
{

Font& load_font(const std::string& name, const std::string& file, int pixel_height)
{
    if (fonts.contains(name)) return fonts.at(name);

    LOG_INFO("FontManager: Loading font {}", name);

    FT_Face face{};
    const std::string path = resource::get_path(file);

    if (FT_New_Face(library, path.c_str(), 0, &face)) LOG_THROW("FreeType: Failed to load font {}", path);

    FT_Set_Pixel_Sizes(face, 0, pixel_height);

    constexpr uint32_t first = 32;
    constexpr uint32_t last = 126;
    constexpr uint32_t padding = 1;

    uint32_t atlas_w = 0;
    uint32_t atlas_h = 0;

    // measure atlas
    for (uint32_t c = first; c <= last; ++c)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;

        atlas_w += face->glyph->bitmap.width + padding;
        atlas_h = std::max(atlas_h, static_cast<uint32_t>(face->glyph->bitmap.rows));
    }

    std::vector<uint8_t> atlas(atlas_w * atlas_h, 0);

    uint32_t x_offset = 0;

    Font font{};
    font.line_height = static_cast<float>(face->size->metrics.height >> 6);
    font.ascent = static_cast<float>(face->size->metrics.ascender >> 6);

    // render glyphs
    for (uint32_t c = first; c <= last; ++c)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;

        FT_GlyphSlot g = face->glyph;

        for (uint32_t y = 0; y < g->bitmap.rows; ++y)
            for (uint32_t x = 0; x < g->bitmap.width; ++x)
                atlas[y * atlas_w + x_offset + x] = g->bitmap.buffer[y * g->bitmap.pitch + x];

        Glyph glyph{};
        glyph.size = {static_cast<float>(g->bitmap.width), static_cast<float>(g->bitmap.rows)};
        glyph.bearing = {static_cast<float>(g->bitmap_left), static_cast<float>(g->bitmap_top)};
        glyph.advance = static_cast<float>(g->advance.x >> 6);

        float u0 = float(x_offset) / float(atlas_w);
        float u1 = float(x_offset + g->bitmap.width) / float(atlas_w);
        float v0 = 0.0f;
        float v1 = float(g->bitmap.rows) / float(atlas_h);

        glyph.uv[0] = {u0, v0};
        glyph.uv[1] = {u1, v0};
        glyph.uv[2] = {u1, v1};
        glyph.uv[3] = {u0, v1};

        font.glyphs[c] = glyph;

        x_offset += g->bitmap.width + padding;
    }

    // Upload texture
    Texture2D tex{};
    tex.name = name;
    tex.width = atlas_w;
    tex.height = atlas_h;

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlas_w, atlas_h, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    add_texture(name, std::move(tex));
    font.texture = &get_texture(name);

    FT_Done_Face(face);

    fonts.emplace(name, std::move(font));
    return fonts.at(name);
}

Font& get_font(const std::string& name)
{
    if (fonts.contains(name)) return fonts.at(name);

    LOG_ERROR("FontManager: Font not found {}", name);
    return fonts.begin()->second;
}
}  // namespace kine::resource
