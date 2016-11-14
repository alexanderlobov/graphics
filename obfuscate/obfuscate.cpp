#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <cmath>
#include <algorithm>
#include <pcl/io/ply_io.h>

typedef pcl::PointXYZRGB Point;
typedef pcl::PointCloud<Point> PointCloud;

class TIsNotTriangle : public std::exception
{
public:
    const char* what() const throw()
    {
        return "Non-triangle faces are not supported";
    }
};

float average(float a, float b, float c)
{
    return (a + b + c) / 3;
}

Point operator+(const Point& a, const Point& b)
{
    Point r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}

Point operator-(const Point& a, const Point& b)
{
    Point r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

Point operator/(const Point& p, double d)
{
    Point r;
    r.x = p.x / d;
    r.y = p.y / d;
    r.z = p.z / d;
    return r;
}

Point operator*(double d, const Point& p)
{
    Point r;
    r.x = d * p.x;
    r.y = d * p.y;
    r.z = d * p.z;
    return r;
}

double norm(const Point& p)
{
    return std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

Point cross_product(const Point& u, const Point& v)
{
    Point r;
    r.x = u.y * v.z - u.z * v.y;
    r.y = u.z * v.x - u.x * v.z;
    r.z = u.x * v.y - u.y * v.x;
    return r;
}

Point CalcM(const Point& a, const Point& b, const Point& c)
{
    Point center = (a + b + c) / 3;

    Point u = b - a;
    Point v = c - a;
    Point t = c - b;

    Point n = cross_product(u, v);
    double l = 0.5 * std::min(norm(u),
                              std::min(norm(v), norm(t)));
    return center - l * n / norm(n);;
}

Point CalcD(Point a, Point b, Point c)
{
    return (a + b) / 2;
}

size_t add_vertex(PointCloud& cloud, const Point& p)
{
    cloud.push_back(p);
    return cloud.size() - 1;
}

void add_index(pcl::Vertices& vertices, size_t index)
{
    vertices.vertices.push_back(index);
}

void add_face(pcl::PolygonMesh& mesh,
              size_t index1,
              size_t index2,
              size_t index3)
{
    mesh.polygons.push_back( pcl::Vertices() );
    pcl::Vertices& vertices = mesh.polygons.back();

    add_index(vertices, index1);
    add_index(vertices, index2);
    add_index(vertices, index3);
}

void transform(const pcl::PolygonMesh& inMesh, pcl::PolygonMesh& outMesh)
{
    outMesh.header = inMesh.header;

    PointCloud cloud;
    pcl::fromPCLPointCloud2(inMesh.cloud, cloud);

    PointCloud newCloud;

    newCloud.reserve(cloud.size());

    typedef std::vector< ::pcl::Vertices> Polygons;

    outMesh.polygons.reserve(inMesh.polygons.size());

    for (Polygons::const_iterator it = inMesh.polygons.begin();
        it != inMesh.polygons.end();
        ++it)
    {
        const int verticesSize = it->vertices.size();
        if (verticesSize != 3) {
            throw TIsNotTriangle();
        }

        const Point& a = cloud[it->vertices[0]];
        const Point& b = cloud[it->vertices[1]];
        const Point& c = cloud[it->vertices[2]];


        Point m = CalcM(a, b, c);
        Point d = CalcD(a, b, c);

        size_t a_index = add_vertex(newCloud, a);
        size_t b_index = add_vertex(newCloud, b);
        size_t c_index = add_vertex(newCloud, c);
        size_t m_index = add_vertex(newCloud, m);
        size_t d_index = add_vertex(newCloud, d);

        add_face(outMesh, a_index, b_index, c_index);

        add_face(outMesh, a_index, m_index, b_index);
        add_face(outMesh, b_index, m_index, c_index);
        add_face(outMesh, c_index, m_index, a_index);
        add_face(outMesh, d_index, m_index, c_index);
    }
    toPCLPointCloud2(newCloud, outMesh.cloud);
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "Usage: obfuscate <input-file> <output-file>\n";
        return EXIT_FAILURE;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    pcl::PolygonMesh inMesh;
    pcl::io::loadPLYFile(inputFile, inMesh);
    pcl::PolygonMesh outMesh;

    transform(inMesh, outMesh);

    pcl::io::savePLYFileBinary(outputFile, outMesh);

    return EXIT_SUCCESS;
}
