#pragma once

#include <GL/glew.h>
#include <string>

void addShader(GLuint shaderProgram,
               const std::string& filename,
               GLenum shaderType);
void linkProgram(GLuint program);
void validateProgram(GLuint program);
void bindAttribute(GLint program, const char* name, GLint& object);
void bindUniform(GLint program, const char* name, GLint& object);

GLuint createProgramChecked();
