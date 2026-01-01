#include "kine/resources/resource_manager.hpp"

#include <stb_image.h>

namespace kine::resource
{

Texture2D& load_texture(const std::string& name, const std::string& file)
{
    LOG_INFO("TextureManager: Loading texture {}", name);

    if (textures.contains(name)) return textures[name];

    const std::string& path = resource::get_path(file);
    textures[name] = load_texture_file(name, path);
    return textures[name];
}

Texture2D& get_texture(const std::string& name)
{
    if (textures.contains(name)) return textures.at(name);

    LOG_ERROR("TextureManager: Failed to load texture {}", name);
    return *error_texture;
}

Texture2D& add_texture(const std::string& name, Texture2D&& tex) { return textures[name] = std::move(tex); }

Texture2D& load_embedded_texture(const std::string& name, const unsigned char* data, const unsigned int len)
{
    if (textures.contains(name)) return textures[name];

    Texture2D tex;
    tex.name = name;

    int channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load_from_memory(data, len, &tex.width, &tex.height, &channels, 0);

    if (!image)
    {
        LOG_ERROR("TextureManager: Failed to load texture from memory {}", name);
        return *error_texture;
    }

    GLenum format = GL_RGB;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glTexImage2D(GL_TEXTURE_2D, 0, format, tex.width, tex.height, 0, format, GL_UNSIGNED_BYTE, image);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    textures[name] = tex;
    return textures[name];
}

Texture2D load_texture_file(const std::string& name, const std::string& path)
{
    Texture2D tex;
    tex.name = name;

    int channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &tex.width, &tex.height, &channels, 0);

    if (!data)
    {
        LOG_ERROR("TextureManager: Failed to load texture {}", path);
        return *error_texture;
    }

    GLenum format = GL_RGB;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glTexImage2D(GL_TEXTURE_2D, 0, format, tex.width, tex.height, 0, format, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

}  // namespace kine::resource
