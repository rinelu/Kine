#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>

namespace kine::resource
{

struct Shader
{
    GLuint program = 0;
};

GLuint compile_shader(GLenum type, const std::string& source);

inline std::unordered_map<std::string, Shader> shaders;

GLuint load_shader_str(const std::string& vert, const std::string& frag);
Shader& load_shader(const std::string& name, const std::string& vertex, const std::string& fragment);
Shader& get_shader(const std::string& name);

}  // namespace kine::resource
