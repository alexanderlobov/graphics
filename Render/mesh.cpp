#include "mesh.h"
#include "gl-utils.h"
#include <pcl/io/ply_io.h>
#include <pcl/PolygonMesh.h>
#include <limits>
#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define ARRAY_END(a) (a + ARRAY_SIZE(a))

struct Box {
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float zmin;
    float zmax;
};

class MeshImpl {
public:
    ~MeshImpl();

    bool loadPLY(const char* filename);
    bool loadCube();
    void render();
    void setMVP(
            float angle,
            const ViewParameters& viewParameters,
            const ProjectionParameters& projectionParameters,
            float rotateYAngle);

private:
    void initCubeVertices();
    void initCubeColors();
    void initCubeElements();

    void initFromPCLMesh(const pcl::PolygonMesh& PCLmesh);
    void initVerticesAndColorsFromPCLMesh(const pcl::PolygonMesh& PCLmesh);
    void initElementsFromPCLMesh(const pcl::PolygonMesh& PCLmesh);

    void initBuffers();
    void initShaders();

    void init();

    glm::vec3 findMeshCenter();
    void findBoundingBox(Box& box);

    std::vector<GLfloat> vertices;
    std::vector<GLfloat> colors;
    std::vector<GLuint> elements;

    GLuint m_vboVertices;
    GLuint m_vboColors;
    GLuint m_iboElements;
    GLint m_attributeCoord;
    GLint m_attributeColor;
    GLint m_uniformMvp;
    GLuint m_program;

    glm::vec3 m_meshCenter;
};

MeshNew::MeshNew()
    : m_impl(new MeshImpl)
{}

MeshNew::~MeshNew()
{}

bool MeshNew::loadPLY(const char* filename)
{
    return m_impl->loadPLY(filename);
}

bool MeshNew::loadCube()
{
    return m_impl->loadCube();
}

void MeshNew::render()
{
    m_impl->render();
}

void MeshNew::setMVP(
        float angle,
        const ViewParameters& viewParameters,
        const ProjectionParameters& projectionParameters,
        float rotateYAngle)
{
    m_impl->setMVP(angle, viewParameters, projectionParameters, rotateYAngle);
}

namespace
{

void updateMinMax(float& min, float& max, float x)
{
    min = std::min(min, x);
    max = std::max(max, x);
}

float average(float x, float y)
{
    return (x + y) / 2;
}

template<typename T>
size_t sizeInBytes(const std::vector<T>& vec)
{
    return sizeof(T) * vec.size();
}

} // anonymous namespace

MeshImpl::~MeshImpl()
{
    glDeleteProgram(m_program);
    glDeleteBuffers(1, &m_vboVertices);
    glDeleteBuffers(1, &m_vboColors);
    glDeleteBuffers(1, &m_iboElements);
}

void MeshImpl::init()
{
    initBuffers();
    initShaders();
    m_meshCenter = findMeshCenter();
}

bool MeshImpl::loadCube()
{
    initCubeVertices();
    initCubeColors();
    initCubeElements();

    init();

    return true;
}

bool MeshImpl::loadPLY(const char* filename)
{
    std::cerr << "Loading model " << filename << '\n';
    pcl::PolygonMesh::Ptr pInputMesh(new pcl::PolygonMesh);
    pcl::io::loadPLYFile(filename, *pInputMesh);
    initFromPCLMesh(*pInputMesh);

    init();

    return true;
}

glm::vec3 MeshImpl::findMeshCenter()
{
    Box box;
    findBoundingBox(box);

    return glm::vec3(
            average(box.xmin, box.xmax),
            average(box.ymin, box.ymax),
            average(box.zmin, box.zmax));
}

void MeshImpl::findBoundingBox(Box& box)
{
    box.xmin = box.ymin = box.zmin = std::numeric_limits<float>::max();
    box.xmax = box.ymax = box.zmax = std::numeric_limits<float>::min();

    int size = vertices.size();
    int i = 0;
    while (i < size) {
        float x = vertices[i++];
        float y = vertices[i++];
        float z = vertices[i++];

        updateMinMax(box.xmin, box.xmax, x);
        updateMinMax(box.ymin, box.ymax, y);
        updateMinMax(box.zmin, box.zmax, z);
    }
}

void MeshImpl::initCubeVertices()
{
    GLfloat cubeVertices[] = {
      // front
      -1.0, -1.0,  1.0,
       1.0, -1.0,  1.0,
       1.0,  1.0,  1.0,
      -1.0,  1.0,  1.0,
      // back
      -1.0, -1.0, -1.0,
       1.0, -1.0, -1.0,
       1.0,  1.0, -1.0,
      -1.0,  1.0, -1.0,
    };
    vertices.assign(cubeVertices, ARRAY_END(cubeVertices));
}

void MeshImpl::initCubeColors()
{
    GLfloat cubeColors[] = {
      // front colors
      1.0, 0.0, 0.0,
      0.0, 1.0, 0.0,
      0.0, 0.0, 1.0,
      1.0, 1.0, 1.0,
      // back colors
      1.0, 0.0, 0.0,
      0.0, 1.0, 0.0,
      0.0, 0.0, 1.0,
      1.0, 1.0, 1.0,
    };
    colors.assign(cubeColors, ARRAY_END(cubeColors));
}

void MeshImpl::initCubeElements()
{
    GLuint cubeElements[] = {
      // front
      0, 1, 2,
      2, 3, 0,
      // top
      1, 5, 6,
      6, 2, 1,
      // back
      7, 6, 5,
      5, 4, 7,
      // bottom
      4, 0, 3,
      3, 7, 4,
      // left
      4, 5, 1,
      1, 0, 4,
      // right
      3, 2, 6,
      6, 7, 3,
    };
    elements.assign(cubeElements, ARRAY_END(cubeElements));
}

void MeshImpl::initVerticesAndColorsFromPCLMesh(const pcl::PolygonMesh& PCLmesh)
{
    typedef pcl::PointCloud<pcl::PointXYZRGB> MyPointCloud;
    MyPointCloud cloud;
    pcl::fromPCLPointCloud2(PCLmesh.cloud, cloud);

    const size_t dataSize = 3 * cloud.size();

    vertices.resize(dataSize);
    colors.resize(dataSize);

    typedef MyPointCloud::const_iterator It;
    size_t indexVertex = 0;
    size_t indexColor = 0;
    for (It it = cloud.begin(); it != cloud.end(); ++it) {
        const pcl::PointXYZRGB& point = *it;
        vertices[indexVertex++] = point.x;
        vertices[indexVertex++] = point.y;
        vertices[indexVertex++] = point.z;
        colors[indexColor++] = float(point.r) / 255;
        colors[indexColor++] = float(point.g) / 255;
        colors[indexColor++] = float(point.b) / 255;
    }
}

void MeshImpl::initElementsFromPCLMesh(const pcl::PolygonMesh& PCLmesh)
{
    elements.resize(PCLmesh.polygons.size() * 3);
    size_t index = 0;

    typedef std::vector<pcl::Vertices>::const_iterator It;
    for (It it = PCLmesh.polygons.begin(); it != PCLmesh.polygons.end(); ++it) {
        const std::vector<uint32_t>& vertices = it->vertices;
        assert(vertices.size() == 3);
        elements[index++] = vertices[0];
        elements[index++] = vertices[1];
        elements[index++] = vertices[2];
    }
}

void MeshImpl::initFromPCLMesh(const pcl::PolygonMesh& PCLmesh)
{
    initVerticesAndColorsFromPCLMesh(PCLmesh);
    initElementsFromPCLMesh(PCLmesh);
}

void MeshImpl::render()
{
    glUseProgram(m_program);
    glEnableVertexAttribArray(m_attributeCoord);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboVertices);
    glVertexAttribPointer(
      m_attributeCoord, // attribute
      3,                 // number of elements per vertex, here (x,y,z)
      GL_FLOAT,          // the type of each element
      GL_FALSE,          // take our values as-is
      0,                 // no extra data between each position
      0                  // offset of first element
    );

    glEnableVertexAttribArray(m_attributeColor);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
    glVertexAttribPointer(
      m_attributeColor, // attribute
      3,                 // number of elements per vertex, here (R,G,B)
      GL_FLOAT,          // the type of each element
      GL_FALSE,          // take our values as-is
      0,                 // no extra data between each position
      0                  // offset of first element
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboElements);
    int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawElements(GL_TRIANGLES, size/sizeof(GLuint), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(m_attributeCoord);
    glDisableVertexAttribArray(m_attributeColor);
}

void MeshImpl::setMVP(
        float angle,
        const ViewParameters& viewParameters,
        const ProjectionParameters& projectionParameters,
        float rotateYAngle)
{
    const glm::mat4 unity(1.0f);
    const glm::vec3 axis_x(1, 0, 0);
    const glm::vec3 axis_y(0, 1, 0);

    glm::mat4 translation = glm::translate(unity, - m_meshCenter);
    glm::mat4 rotation = glm::rotate(unity, glm::radians(180.0f), axis_x) *
                         glm::rotate(unity, glm::radians(rotateYAngle), axis_y);

    glm::mat4 anim = glm::rotate(unity, glm::radians(angle), axis_y);

    glm::mat4 model = anim * rotation * translation;

    glm::mat4 vp = calculateViewProjectionMatrix(
            viewParameters,
            projectionParameters);

    glm::mat4 mvp = vp * model;

    glUseProgram(m_program);
    glUniformMatrix4fv(m_uniformMvp, 1, GL_FALSE, glm::value_ptr(mvp));
}

void MeshImpl::initBuffers()
{
    glGenBuffers(1, &m_vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeInBytes(vertices),
                 vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_vboColors);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
    glBufferData(GL_ARRAY_BUFFER, sizeInBytes(colors),
                colors.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_iboElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes(elements),
                 elements.data(), GL_STATIC_DRAW);
}

void MeshImpl::initShaders()
{
    m_program = createProgramChecked();

    addShader(m_program, "shader.vs", GL_VERTEX_SHADER);
    addShader(m_program, "shader.fs", GL_FRAGMENT_SHADER);

    linkProgram(m_program);
    validateProgram(m_program);

    bindAttribute(m_program, "coord", m_attributeCoord);
    bindAttribute(m_program, "color", m_attributeColor);
    bindUniform(m_program, "mvp", m_uniformMvp);

    glUseProgram(m_program);
}
