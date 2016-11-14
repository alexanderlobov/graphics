#pragma once

#include <GL/glew.h>
#include <string>

class ViewParameters;
class ProjectionParameters;

class ISkybox {
public:
    virtual bool load(const std::string& path) = 0;
    virtual void render() = 0;
    virtual void setMVP(
            float angle,
            const ViewParameters& viewParameters,
            const ProjectionParameters& projectionParameters) = 0;
};

class Skybox : public ISkybox {
public:
    ~Skybox();

    bool load(const std::string& path);
    void render();
    void setMVP(
            float angle,
            const ViewParameters& viewParameters,
            const ProjectionParameters& projectionParameters);

private:
    void loadProgram();
    void initVertices();
    void initTextures(const std::string& path);

    GLuint m_vbo;
    GLint m_attributeCoord;
    GLint m_uniformMvp;
    GLuint m_program;
    GLuint m_textureID;
};

class EmptySkybox : public ISkybox {
public:
    bool load(const std::string& path);
    void render();
    void setMVP(
            float angle,
            const ViewParameters& viewParameters,
            const ProjectionParameters& projectionParameters);
};
