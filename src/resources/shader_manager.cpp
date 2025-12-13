#include "shader_manager.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "log.hpp"
#include "resource_manager.hpp"

namespace kine
{

ShaderManager::ShaderManager(ResourceManager& rm) : resources(rm) {}

ShaderManager::~ShaderManager()
{
    for (auto& [_, shader] : shaders) glDeleteProgram(shader.program);
}

Shader& ShaderManager::load(const std::string& name, const std::string& vertex, const std::string& fragment)
{
    LOG_INFO("ShaderManager: Loading shader : ", name);
    if (shaders.contains(name)) return shaders[name];

    std::string vert_src = read_file(resources.get_path(vertex));
    std::string frag_src = read_file(resources.get_path(fragment));

    GLuint vs = compile(GL_VERTEX_SHADER, vert_src);
    GLuint fs = compile(GL_FRAGMENT_SHADER, frag_src);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    shaders[name] = {program};
    return shaders[name];
}

Shader& ShaderManager::get(const std::string& name) { return shaders.at(name); }

std::string ShaderManager::read_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file) LOG_THROW(std::runtime_error, "ShaderManager: Failed to open shader: ", path);

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint ShaderManager::compile(GLenum type, const std::string& src)
{
    GLuint shader = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(shader, 1, &c, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        throw std::runtime_error(log);
    }

    return shader;
}

}  // namespace kine
