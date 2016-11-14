#include <boost/numeric/ublas/vector.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/exception/all.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <limits>

using boost::numeric::ublas::c_vector;
using boost::lexical_cast;
using boost::optional;
using std::string;

typedef c_vector<float, 3> Vector3f;
typedef c_vector<float, 2> Vector2f;
typedef size_t Index;

struct Vertex {
    Index index;
    optional<Index> textureIndex;
    optional<Index> normalIndex;
};

typedef std::vector<Vertex> Vertices;
typedef std::vector<std::string> Tokens;
typedef std::vector<int> IndexMap;

class Mesh {
public:
    void readOBJ(const std::string& filename);
    void writeOBJ(const std::string& filename);
    void swap(Mesh& m);

    const Vector3f& getVertexCoord(Index index) const;

private:
    void parseLine(std::string line);
    void parseVertex(const Tokens& tokens);
    void parseVertexTexture(const Tokens& tokens);
    void parseNormal(const Tokens& tokens);
    void parseFace(const Tokens& tokens);
    void parseMtllib(const Tokens& tokens);
    void parseUsemtl(const Tokens& tokens);

    void writeCoords(std::ostream& s);
    void writeNormals(std::ostream& s);
    void writeTextureCoords(std::ostream& s);
    void writeFaces(std::ostream& s);

public:
    std::vector<Vector3f> m_coords;
    std::vector<Vector2f> m_textureCoords;
    std::vector<Vector3f> m_normalCoords;
    std::vector<Vertices> m_faces;
    optional<std::vector<Vector3f> > m_colors;

    optional<std::string> m_mtllib;
    optional<std::string> m_usemtl;
};

struct Options {
    string inputFilename;
    string outputFilename;
    double copyingFraction;
    double addingFraction;
    int vertexAddingModulo;
    int vertexCopyingModulo;
};

std::runtime_error parsingError()
{
    return std::runtime_error("OBJ parsing error");
}

std::runtime_error notImplemented()
{
    return std::runtime_error("Not implemented");
}

#define PRECONDITION(expression) \
    if (! (expression)) { \
        std::cerr << "Precondition failed in file " << __FILE__ \
                  << " at line " << __LINE__ << std::endl; \
        throw parsingError(); \
    }

void Mesh::readOBJ(const std::string& filename)
{
    std::ifstream fs(filename.c_str());
    std::string line;
    while (std::getline(fs, line)) {
        parseLine(line);
    }
}

void cutIfLast(std::string& s, char c)
{
    if (s.size() > 0) {
        const char back = s[s.size() - 1];
        if (back == c) {
            s.resize(s.size() - 1);
        }
    }
}

void Mesh::parseLine(std::string line)
{
    using boost::starts_with;
    using boost::is_any_of;
    using boost::split;

    // TODO: remove this workaround
    cutIfLast(line, '\r');

    if (line.empty() ) {
        return;
    }

    if (!boost::starts_with(line, "#")) {
        Tokens tokens;
        boost::split(tokens, line, is_any_of("\t "), boost::token_compress_on);
        const std::string first = tokens.front();
        if (first == "v") {
            parseVertex(tokens);
        } else if (first == "vt") {
            parseVertexTexture(tokens);
        } else if (first == "vn") {
            parseNormal(tokens);
        } else if (first == "f") {
            parseFace(tokens);
        } else if (first == "mtllib") {
            parseMtllib(tokens);
        } else if (first == "usemtl") {
            parseUsemtl(tokens);
        } else {
            throw std::runtime_error("unknown line type\n" + line);
        }
    }
}

Vector3f readVector3f(const Tokens& tokens, int from = 1)
{
    PRECONDITION(tokens.size() > 3);

    Vector3f v;
    for (int i = 0; i < 3; ++i) {
        v[i] = lexical_cast<float>(tokens[i+from]);
    }
    return v;
}

void Mesh::parseVertex(const Tokens& tokens)
{
    m_coords.push_back(readVector3f(tokens));

    if (tokens.size() == 7) { // v x y z r g b
        throw notImplemented();
        if (!m_colors) {
            m_colors = std::vector<Vector3f>();
        }
        m_colors->push_back(readVector3f(tokens, 4));
    }
}

void Mesh::parseVertexTexture(const Tokens& tokens)
{
    PRECONDITION(tokens.size() > 2);

    Vector2f v;
    for (int i = 0; i < 2; ++i) {
        v[i] = lexical_cast<float>(tokens[i+1]);
    }
    m_textureCoords.push_back(v);
}

void Mesh::parseNormal(const Tokens& tokens)
{
    m_normalCoords.push_back(readVector3f(tokens));
}

void Mesh::parseFace(const Tokens& tokens)
{
    PRECONDITION(tokens.size() > 1);
    m_faces.push_back(Vertices());
    Vertices& vertices = m_faces.back();
    for (Tokens::const_iterator it = ++tokens.begin(); it != tokens.end(); ++it)
    {
        Tokens indicesStr;
        split(indicesStr, *it, boost::is_any_of("/"));

        Vertex vertex;
        vertex.index = lexical_cast<Index>(indicesStr[0]) - 1;
        if (indicesStr.size() > 1) {
            vertex.textureIndex = lexical_cast<Index>(indicesStr[1]) - 1;
            if (indicesStr.size() > 2) {
                vertex.normalIndex = lexical_cast<Index>(indicesStr[2]) - 1;
            }
        }

        vertices.push_back(vertex);
    }
}

void Mesh::parseMtllib(const Tokens& tokens)
{
    PRECONDITION(tokens.size() > 1);
    m_mtllib = tokens[1];
}

void Mesh::parseUsemtl(const Tokens& tokens)
{
    PRECONDITION(tokens.size() > 1);
    m_usemtl = tokens[1];
}

void Mesh::writeOBJ(const std::string& filename)
{
    std::ofstream fs(filename.c_str());
    if (m_mtllib) {
        fs << "mtllib " << *m_mtllib << "\n\n";
    }
    writeCoords(fs);
    writeNormals(fs);
    writeTextureCoords(fs);
    if (m_usemtl) {
        fs << '\n' << "usemtl " << *m_usemtl << '\n';
    }
    writeFaces(fs);
}

void Mesh::writeCoords(std::ostream& s)
{
    PRECONDITION(!m_colors || m_colors->size() == m_coords.size());
    for (size_t i = 0; i < m_coords.size(); ++i) {
        const Vector3f& v = m_coords[i];
        s << "v " << v[0] << ' ' << v[1] << ' ' << v[2];
        if (m_colors) {
            const Vector3f& c = (*m_colors)[i];
            s << ' ' << c[0] << ' ' << c[1] << ' ' << c[2];
        }
        s << '\n';
    }
}

void Mesh::writeNormals(std::ostream& s)
{
    for (size_t i = 0; i < m_normalCoords.size(); ++i) {
        const Vector3f& v = m_normalCoords[i];
        s << "vn" << v[0] << ' ' << v[1] << ' ' << v[2];
    }
    s << '\n';
}

void Mesh::writeTextureCoords(std::ostream& s)
{
    for (size_t i = 0; i < m_textureCoords.size(); ++i) {
        const Vector2f& v = m_textureCoords[i];
        s << "vt " << v[0] << ' ' << v[1] << '\n';
    }
}

void Mesh::writeFaces(std::ostream& s)
{
    for (size_t i = 0; i < m_faces.size(); ++i) {
        s << "f";
        const Vertices& vertices = m_faces[i];
        for (size_t j = 0; j < vertices.size(); ++j) {
            const Vertex& vertex = vertices[j];
            s << ' ' << vertex.index + 1;
            if (vertex.textureIndex) {
                s << '/' << *vertex.textureIndex + 1;
                if (vertex.normalIndex) {
                    s << '/' << *vertex.normalIndex + 1;
                }
            }
        }
        s << '\n';
    }
}

void Mesh::swap(Mesh& m)
{
    std::swap(m_coords, m.m_coords);
    std::swap(m_textureCoords, m.m_textureCoords);
    std::swap(m_normalCoords, m.m_normalCoords);
    std::swap(m_faces, m.m_faces);
    std::swap(m_mtllib, m.m_mtllib);
    std::swap(m_usemtl, m.m_usemtl);
}

const Vector3f& Mesh::getVertexCoord(Index index) const
{
    return m_coords[index];
}

Vector3f cross_product(const Vector3f& u, const Vector3f& v)
{
    Vector3f r;
    r[0] = u[1] * v[2] - u[2] * v[1];
    r[1] = u[2] * v[0] - u[0] * v[2];
    r[2] = u[0] * v[1] - u[1] * v[0];
    return r;
}

optional<Vector3f> CalcM(
        const Vector3f& a,
        const Vector3f& b,
        const Vector3f& c)
{
    using boost::numeric::ublas::norm_2;

    Vector3f center = (a + b + c) / 3;

    Vector3f u = b - a;
    Vector3f v = c - a;
    Vector3f t = c - b;

    Vector3f n = cross_product(u, v);
    double l = 0.5 * std::min(norm_2(u),
                              std::min(norm_2(v), norm_2(t)));
    double normN = norm_2(n);

    if (normN <= std::numeric_limits<double>::min())
        return boost::none;
    else
        return Vector3f(center - l * n / norm_2(n));
}

Vector3f CalcD(const Vector3f& a, const Vector3f& b, const Vector3f& c)
{
    return (a + b) / 2;
}

Index addVertexCoord(Mesh& mesh, const Vector3f& v)
{
     mesh.m_coords.push_back(v);
     return mesh.m_coords.size() - 1;
}

bool shouldCopyVertex(const Options& options, int vertexCopyCounter)
{
    return options.vertexCopyingModulo != std::numeric_limits<int>::max()
           && vertexCopyCounter % options.vertexCopyingModulo == 0;
}

Index addVertex(
        Mesh& mesh,
        const Mesh& inMesh,
        Index oldIndex,
        IndexMap& oldIndexToNew,
        int& vertexCopyCounter,
        const Options& options)
{
    if (oldIndexToNew.at(oldIndex) == -1) {
        return oldIndexToNew[oldIndex] = addVertexCoord(
                mesh, inMesh.getVertexCoord(oldIndex));
    } else {
        if (shouldCopyVertex(options, vertexCopyCounter++)) {
            return addVertexCoord(mesh, inMesh.getVertexCoord(oldIndex));
        } else {
            return oldIndexToNew[oldIndex];
        }
    }
}

Vertex copyVertex(Mesh& mesh, const Mesh& inMesh, const Vertex& oldVertex,
                  IndexMap& oldIndexToNew, int& vertexCopyCounter, const Options&
                  options)
{
    Vertex newVertex(oldVertex);
    newVertex.index = addVertex(
            mesh, inMesh, oldVertex.index, oldIndexToNew, vertexCopyCounter, options);
    return newVertex;
}

void add_face(Mesh& mesh,
              const Vertex& vertex1,
              const Vertex& vertex2,
              const Vertex& vertex3)
{
    mesh.m_faces.push_back(Vertices());
    Vertices& vertices = mesh.m_faces.back();

    vertices.push_back(vertex1);
    vertices.push_back(vertex2);
    vertices.push_back(vertex3);
}

bool shouldAddVertices(const Options& options, int faceCounter)
{
    return options.vertexAddingModulo != std::numeric_limits<int>::max()
           && faceCounter % options.vertexAddingModulo == 0;
}

std::ostream& operator<<(std::ostream& s, const Vector3f v)
{
    return s << v[0] << ' ' << v[1] << ' ' << v[2];
}

void addNewVertices(
        Mesh& mesh,
        const Vertex& vertexA, const Vertex& vertexB, const Vertex& vertexC)
{
    const Vector3f& a = mesh.getVertexCoord(vertexA.index);
    const Vector3f& b = mesh.getVertexCoord(vertexB.index);
    const Vector3f& c = mesh.getVertexCoord(vertexC.index);

    optional<Vector3f> m = CalcM(a, b, c);

    if (!m) {
        return;
    }

    Vector3f d = CalcD(a, b, c);

    Vertex vertexM;
    Vertex vertexD;

    vertexM.index = addVertexCoord(mesh, *m);
    vertexD.index = addVertexCoord(mesh, d);

    if (vertexA.textureIndex) {
        vertexM.textureIndex = 0;
        vertexD.textureIndex = 0;
    }

    add_face(mesh, vertexA, vertexM, vertexB);
    add_face(mesh, vertexB, vertexM, vertexC);
    add_face(mesh, vertexC, vertexM, vertexA);
    add_face(mesh, vertexD, vertexM, vertexC);
}

void transform(const Mesh& inMesh, Mesh& outMesh, const Options& options)
{
    // TODO: implement copying of colors
    Mesh mesh;
    typedef std::vector<Vertices>::const_iterator It;

    int faceCounter = 0;
    int vertexCopyCounter = 0;
    IndexMap oldIndexToNew(inMesh.m_coords.size(), -1);

    for (It it = inMesh.m_faces.begin(); it != inMesh.m_faces.end(); ++it) {
        PRECONDITION(it->size() == 3);

        const Vertices& vertices = *it;

        Vertex vertexA = copyVertex(mesh, inMesh, vertices[0], oldIndexToNew,
                vertexCopyCounter, options);
        Vertex vertexB = copyVertex(mesh, inMesh, vertices[1], oldIndexToNew,
                vertexCopyCounter, options);
        Vertex vertexC = copyVertex(mesh, inMesh, vertices[2], oldIndexToNew,
                vertexCopyCounter, options);

        add_face(mesh, vertexA, vertexB, vertexC);

        if (shouldAddVertices(options, faceCounter)) {
            addNewVertices(mesh, vertexA, vertexB, vertexC);
        }

        ++faceCounter;
    }

    mesh.m_textureCoords = inMesh.m_textureCoords;
    mesh.m_normalCoords = inMesh.m_normalCoords;
    mesh.m_mtllib = inMesh.m_mtllib;
    mesh.m_usemtl = inMesh.m_usemtl;
    // mesh.m_colors = inMesh.m_colors;

    outMesh.swap(mesh);
}

int fractionToModulo(double fraction)
{
    if (fraction < 0.0001)
        return std::numeric_limits<int>::max();
    else
        return 100.0 / fraction;
}

bool initOptions(Options& options, int argc, char** argv)
{
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()
        ("help", "Print help message")
        ("input-file",
         po::value(&options.inputFilename)->required(),
         "Input filename")
        ("output-file",
         po::value(&options.outputFilename)->required(),
         "Output filename")
        ("copy-fraction",
         po::value(&options.copyingFraction)->default_value(1.0),
         "Copying fraction, in percents")
        ("adding-fraction",
         po::value(&options.addingFraction)->default_value(1.0),
         "Adding fraction, in percents")
        ;

    po::positional_options_description p;
    p.add("input-file", 1).add("output-file", 1);

    po::variables_map vm;

    try {
        po::store(po::command_line_parser(argc, argv)
                    .options(desc)
                    .positional(p)
                    .run(),
                  vm);

        if (vm.count("help")) {
            std::cout << desc << '\n';
            return false;
        }

        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    options.vertexAddingModulo = fractionToModulo(options.addingFraction);
    options.vertexCopyingModulo = fractionToModulo(options.copyingFraction);

    return true;
}

int main(int argc, char** argv)
{
    Options options;
    if (!initOptions(options, argc, argv))
        return -1;

    std::cout << "input-file: " << options.inputFilename << '\n'
              << "output-file: " << options.outputFilename << '\n';

    Mesh inputMesh;
    inputMesh.readOBJ(options.inputFilename);

    Mesh outputMesh;
    transform(inputMesh, outputMesh, options);

    outputMesh.writeOBJ(options.outputFilename);

    return 0;
}
