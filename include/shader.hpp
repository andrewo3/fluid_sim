#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <GL/glew.h>
#include <vector>

class Shader {
    public:
        Shader(GLenum typ);
        bool init(std::string filename);
        GLuint getID() {
            return id;
        }
        GLuint id;
        GLenum type;
};

class ShaderProgram {
    public:
        bool init();
        void attachShader(Shader* shader);
        void detachShader(Shader* shader);
        bool link();
        std::vector<Shader*> shaders;
        GLuint id;
};

void printShaderLog(GLuint shader);
void printProgramLog(GLuint program);

#endif