#include "gl-utils.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>

bool readFile(const char* filename, std::string& out)
{
    std::ifstream in(filename, std::ios::in);
    if (in) {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        out = contents.str();
        return true;
    }
    return false;
}

std::string readFile(const char* filename)
{
    std::string out;
    if (readFile(filename, out)) {
        return out;
    } else {
        std::cerr << "Can't open file " << filename << '\n';
        exit(EXIT_FAILURE);
    }
}

template <typename Func>
std::string getInfoLog(GLuint object, Func func)
{
    static const size_t LOG_SIZE = 4096;
    GLchar infoLog[LOG_SIZE];
    GLsizei length;

    func(object, LOG_SIZE, &length, infoLog);

    return infoLog;
}

std::string getShaderInfoLog(GLuint shaderObject)
{
    return getInfoLog(shaderObject, glGetShaderInfoLog);
}

std::string getProgramInfoLog(GLuint programObject)
{
    return getInfoLog(programObject, glGetProgramInfoLog);
}

void checkShaderCompilation(GLuint shaderObject,
                            GLenum shaderType,
                            const std::string& filename)
{
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (0 == status) {
        std::cerr << "Error compiling shader file " << filename
                  << " with type " << shaderType
                  << ": " << getShaderInfoLog(shaderObject) << '\n';
        exit(EXIT_FAILURE);
    }
}

void checkProgramStatus(GLuint program, GLenum name, const std::string message)
{
    GLint status;
    glGetProgramiv(program, name, &status);
    if (0 == status) {
        std::cerr << message << ": '"
                  << getProgramInfoLog(program) << "'\n";
        exit(EXIT_FAILURE);
    }
}

void linkProgram(GLuint program)
{
    glLinkProgram(program);
    checkProgramStatus(program, GL_LINK_STATUS, "Can't link shader program");
}

void validateProgram(GLuint program)
{
    glValidateProgram(program);
    checkProgramStatus(program, GL_VALIDATE_STATUS,
                       "Can't validate shader program");
}

void bindAttribute(GLint program, const char* name, GLint& object)
{
    object = glGetAttribLocation(program, name);
    if (-1 == object) {
        std::cerr << "Could not bind attribute " << name << '\n';
        exit(EXIT_FAILURE);
    }
}

void bindUniform(GLint program, const char* name, GLint& object)
{
    object = glGetUniformLocation(program, name);
    if (-1 == object) {
        std::cerr << "Could not bind uniform " << name << '\n';
        exit(EXIT_FAILURE);
    }
}
void addShader(GLuint shaderProgram,
               const std::string& filename,
               GLenum shaderType)
{
    const GLuint shaderObj = glCreateShader(shaderType);
    const std::string& shaderText = readFile(filename.c_str());

    if (0 == shaderObj) {
        std::cerr << "Error creating shader of type " << shaderType << '\n';
        exit(EXIT_FAILURE);
    }

    const size_t numShaders = 1;
    const GLchar* pString[numShaders];
    GLint pLength[numShaders];

    pString[0] = shaderText.c_str();
    pLength[0] = shaderText.length();

    glShaderSource(shaderObj, numShaders, pString, pLength);
    glCompileShader(shaderObj);
    checkShaderCompilation(shaderObj, shaderType, filename);
    glAttachShader(shaderProgram, shaderObj);
}

GLuint createProgramChecked()
{
    const GLuint program = glCreateProgram();
    if (0 == program) {
        std::cerr << "Error creating shader program\n";
        exit(EXIT_FAILURE);
    }
    return program;
}
