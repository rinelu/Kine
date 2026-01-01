#include <stdexcept>
#include "resources/resource_manager.hpp"

namespace kine ::resource
{

GLuint load_shader_str(const std::string& vert, const std::string& frag)
{
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vert);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, frag);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

Shader& load_shader(const std::string& name, const std::string& vertex, const std::string& fragment)
{
    LOG_INFO("ShaderManager: Loading shader {}", name);
    if (shaders.contains(name)) return shaders[name];

    std::string vert_src = read_file(resource::get_path(vertex));
    std::string frag_src = read_file(resource::get_path(fragment));

    GLuint program = load_shader_str(vert_src, frag_src);

    shaders[name] = {program};
    return shaders[name];
}

Shader& get_shader(const std::string& name) { return shaders.at(name); }

GLuint compile_shader(GLenum type, const std::string& src)
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

}  // namespace kine::resource
