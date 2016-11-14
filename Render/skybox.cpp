#include "skybox.h"
#include "gl-utils.h"
#include "image.h"
#include "transform.h"
#include <iostream>
#include <string>
#include <vector>
#include <exception>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

void Skybox::loadProgram()
{
    // std::cerr << "Loading skybox shaders\n";
    m_program = createProgramChecked();

    addShader(m_program, "skybox.vs", GL_VERTEX_SHADER);
    addShader(m_program, "skybox.fs", GL_FRAGMENT_SHADER);

    linkProgram(m_program);
    validateProgram(m_program);

    bindAttribute(m_program, "coord", m_attributeCoord);
    bindUniform(m_program, "mvp", m_uniformMvp);
}

bool Skybox::load(const std::string& path)
{
    loadProgram();
    initVertices();
    initTextures(path);

    return true;
}

void Skybox::initVertices()
{
    // std::cerr << "Loading skybox vertices\n";
    float points[] = {
      -10.0f,  10.0f, -10.0f,
      -10.0f, -10.0f, -10.0f,
       10.0f, -10.0f, -10.0f,
       10.0f, -10.0f, -10.0f,
       10.0f,  10.0f, -10.0f,
      -10.0f,  10.0f, -10.0f,
      
      -10.0f, -10.0f,  10.0f,
      -10.0f, -10.0f, -10.0f,
      -10.0f,  10.0f, -10.0f,
      -10.0f,  10.0f, -10.0f,
      -10.0f,  10.0f,  10.0f,
      -10.0f, -10.0f,  10.0f,
      
       10.0f, -10.0f, -10.0f,
       10.0f, -10.0f,  10.0f,
       10.0f,  10.0f,  10.0f,
       10.0f,  10.0f,  10.0f,
       10.0f,  10.0f, -10.0f,
       10.0f, -10.0f, -10.0f,
       
      -10.0f, -10.0f,  10.0f,
      -10.0f,  10.0f,  10.0f,
       10.0f,  10.0f,  10.0f,
       10.0f,  10.0f,  10.0f,
       10.0f, -10.0f,  10.0f,
      -10.0f, -10.0f,  10.0f,
      
      -10.0f,  10.0f, -10.0f,
       10.0f,  10.0f, -10.0f,
       10.0f,  10.0f,  10.0f,
       10.0f,  10.0f,  10.0f,
      -10.0f,  10.0f,  10.0f,
      -10.0f,  10.0f, -10.0f,
      
      -10.0f, -10.0f, -10.0f,
      -10.0f, -10.0f,  10.0f,
       10.0f, -10.0f, -10.0f,
       10.0f, -10.0f, -10.0f,
      -10.0f, -10.0f,  10.0f,
       10.0f, -10.0f,  10.0f
    };
    glGenBuffers (1, &m_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, m_vbo);
    glBufferData (GL_ARRAY_BUFFER, 3 * 36 * sizeof (float), &points, GL_STATIC_DRAW);
}

bool load_cube_map_side (
  GLuint texture,
  GLenum side_target,
  const char* file_name)
{
    glBindTexture (GL_TEXTURE_CUBE_MAP, texture);

    std::vector<unsigned char> rgbBuffer;
    int width = 0;
    int height = 0;

    if (!readJPEGtoRGB(file_name, rgbBuffer, width, height))
        return false;

    // std::cerr << "File " << file_name << " was read\n""
    //              "width = " << width
    //           << "\nheight = " << height
    //           << "\nsize = " << rgbBuffer.size()
    //           << "\n";

    glTexImage2D (
      side_target,
      0,
      GL_RGB,
      width,
      height,
      0,
      GL_RGB,
      GL_UNSIGNED_BYTE,
      rgbBuffer.data()
    );

    return true;
}

namespace
{

class TextureInitError : public std::exception {
public:
    const char* what() const throw ()
    {
        return "Can't set up a texture. Golaktiko opastnoste!";
    }
};

void check(bool value)
{
    if (!value)
        throw TextureInitError();
}

} // anonymous namespace

GLuint initTexturesImpl(
    const char* posx,
    const char* negx,
    const char* posy,
    const char* negy,
    const char* posz,
    const char* negz)
{
    GLuint texture;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures (1, &texture);
  
    // load each image and copy into a side of the cube-map texture
    check (
      load_cube_map_side (texture, GL_TEXTURE_CUBE_MAP_POSITIVE_X, posx));
    check (
      load_cube_map_side (texture, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, negx));

    check (
      load_cube_map_side (texture, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, posy));
    check (
      load_cube_map_side (texture, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, negy));

    check (
      load_cube_map_side (texture, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, posz));
    check (
      load_cube_map_side (texture, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, negz));

    // format cube map texture
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return texture;
}

typedef std::vector<std::string> Filenames;

namespace
{

Filenames generateFullNames(const std::string& pathString)
{
    const char* names[] = {
        "posx.jpg",
        "negx.jpg",
        "posy.jpg",
        "negy.jpg",
        "posz.jpg",
        "negz.jpg",
        0
    };

    Filenames fullNames;

    fs::path directoryPath(pathString);
    for (const char** nameIt = names; *nameIt; ++nameIt) {
        fs::path path(directoryPath / *nameIt);
        fullNames.push_back( path.string() );
    }

    return fullNames;
}

} // anonymous namespace

void Skybox::initTextures(const std::string& path)
{
    Filenames filenames = generateFullNames(path);
    assert(filenames.size() == 6);
    m_textureID = initTexturesImpl(
            filenames[0].c_str(),
            filenames[1].c_str(),
            filenames[2].c_str(),
            filenames[3].c_str(),
            filenames[4].c_str(),
            filenames[5].c_str()
    );
}

void Skybox::render()
{
    glDepthMask(GL_FALSE);
    glUseProgram(m_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

    glEnableVertexAttribArray (m_attributeCoord);
    glBindBuffer (GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer (m_attributeCoord, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
}

void Skybox::setMVP(
        float angle,
        const ViewParameters& viewParameters,
        const ProjectionParameters& projectionParameters)
{
    // glm::mat4 translation = glm::translate(
    //         glm::mat4(1.0f),
    //         -viewParameters.eye);

    glm::vec3 axis_y(0, 1, 0);
    glm::mat4 anim = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis_y);

    glm::mat4 vp = calculateViewProjectionMatrix(
            viewParameters,
            projectionParameters);

    // glm::mat4 mvp = vp * anim * translation;
    glm::mat4 mvp = vp * anim;

    glUseProgram(m_program);
    glUniformMatrix4fv(m_uniformMvp, 1, GL_FALSE, glm::value_ptr(mvp));
}

Skybox::~Skybox()
{
    glDeleteProgram(m_program);
    glDeleteBuffers(1, &m_vbo);
}

bool EmptySkybox::load(const std::string& path)
{
    return true;
}

void EmptySkybox::render()
{}

void EmptySkybox::setMVP(
        float angle,
        const ViewParameters& viewParameters,
        const ProjectionParameters& projectionParameters)
{}
