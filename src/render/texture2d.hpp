#pragma once
#include <glad/glad.h>
#include <string>

namespace kine
{

struct Texture2D
{
    GLuint id = 0;
    int width = 0;
    int height = 0;
    std::string name;
};

}  // namespace kine
