#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>

namespace kine
{

class ResourceManager;

struct Shader
{
    GLuint program = 0;
};

class ShaderManager
{
   public:
    explicit ShaderManager(ResourceManager& rm);
    ~ShaderManager();

    Shader& load(const std::string& name, const std::string& vertex, const std::string& fragment);

    Shader& get(const std::string& name);

   private:
    std::string read_file(const std::string& path);
    GLuint compile(GLenum type, const std::string& source);

    ResourceManager& resources;
    std::unordered_map<std::string, Shader> shaders;
};

}  // namespace kine
