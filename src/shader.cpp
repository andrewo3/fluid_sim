#include "shader.hpp"
#include <string>
#include <gl/glew.h>
#include <gl/GLU.h>
#include <fstream>
#include <algorithm>

void printShaderLog( GLuint shader )
{
    //Make sure name is shader
    if(glIsShader(shader))
    {
        //Shader log length
        int infoLogLength = 0;
        int maxLength = infoLogLength;
        
        //Get info string length
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        
        //Allocate string
        char* infoLog = new char[maxLength];
        
        //Get info log
        glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
        if(infoLogLength > 0) {
            //Print Log
            printf("%s\n", infoLog);
        }

        //Deallocate string
        delete[] infoLog;
    }
    else
    {
        printf( "Name %d is not a shader\n", shader );
    }
}

Shader::Shader(GLenum typ) {
    type = typ;
}

bool Shader::init(std::string filename) {
    std::ifstream file;
    file.open(filename, std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;
    file.seekg(0, file.end);
    int sz = file.tellg();
    file.seekg(0, file.beg);
    char* buffer = new char[sz+1];
    file.read(buffer, sz);
    file.close();
    buffer[sz] = '\0';
    id = glCreateShader(type);
    glShaderSource(id, 1, &buffer, &sz);
    glCompileShader(id);
    delete[] buffer;

    //Check vertex shader for errors
    GLint vShaderCompiled = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &vShaderCompiled);
    if(vShaderCompiled != GL_TRUE)
    {
        return false;
    }
    return true;
}

bool ShaderProgram::init() {
    id = glCreateProgram();
    if (id == 0) {
        return false;
    }
    return true;
}

void ShaderProgram::attachShader(Shader* shader) {
    glAttachShader(id,shader->id);
    shaders.push_back(shader);

}

void ShaderProgram::detachShader(Shader* shader) {
    glDetachShader(id,shader->id);
    shaders.erase(std::find(shaders.begin(),shaders.end(),shader));
}

bool ShaderProgram::link() {
    //Link program
    glLinkProgram(id);

    //Check for errors
    GLint programSuccess = GL_TRUE;
    glGetProgramiv(id, GL_LINK_STATUS, &programSuccess);
    if(programSuccess != GL_TRUE)
    {
        return false;
    }
    return true;
}