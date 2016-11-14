#include "image.h"
#include "mesh.h"
#include "skybox.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <boost/scoped_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#define GLM_FORCE_RADIANS
#include <glm/trigonometric.hpp>

boost::scoped_ptr<ISkybox> gSkybox1;
boost::scoped_ptr<ISkybox> gSkybox2;
boost::scoped_ptr<ISkybox> gEmptySkybox;

ViewParameters gViewParameters;
ProjectionParameters gProjectionParameters;

namespace fs = boost::filesystem;

struct Options
{
    std::string inputDirectory;
    std::string outputDirectory;
    std::string skybox1Directory;
    std::string skybox2Directory;
    std::string skybox1Name;
    std::string skybox2Name;
    std::string noSkyboxName;
    bool isCubeModel;
    int screenWidth;
    int screenHeight;
    int pictureQty;
    float fovyDegrees;
    float initialAngleDegrees;
    float eyeX;
    float eyeY;
    float eyeZ;
    float centerX;
    float centerY;
    float centerZ;

};

Options gOptions;

void setParams(MeshNew& mesh, int pictureNumber, int totalPictureQty, ISkybox& skybox) {
    float angle = 360.0f * pictureNumber / totalPictureQty;

    mesh.setMVP(angle, gViewParameters, gProjectionParameters,
                gOptions.initialAngleDegrees);
    skybox.setMVP(angle, gViewParameters, gProjectionParameters);
}

std::string generateFilename(int i)
{
    std::stringstream s;
    s << std::setfill('0') << std::setw(4) << i
      << std::setw(0) << ".jpg";
    return s.str();
}

void saveImage(int i, const fs::path& outpath)
{
    static std::vector<GLubyte> image(
            gOptions.screenWidth * gOptions.screenHeight * 3, 1);

    glReadPixels(0, 0, gOptions.screenWidth, gOptions.screenHeight,
                 GL_RGB, GL_UNSIGNED_BYTE, image.data());

    fs::path path = outpath / generateFilename(i);

    std::cerr << "Writing a file " << path << '\n';

    if (!saveRGBtoJPEG(
                image.data(), gOptions.screenWidth,
                gOptions.screenHeight, path.string().c_str()))
    {
        std::cerr << "Can't write file " << path << '\n';
    }
}

void draw(MeshNew& mesh, ISkybox& skybox)
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    skybox.render();
    mesh.render();
}

void render(
        MeshNew& mesh,
        ISkybox& skybox,
        int pictureQty,
        const fs::path& outpath)
{
    fs::create_directory(outpath);
    for (int i = 0; i < pictureQty; ++i) {
        setParams(mesh, i, pictureQty, skybox);
        draw(mesh, skybox);
        saveImage(i, outpath);
        glutSwapBuffers();
    }
}

void renderMesh(MeshNew& mesh, const std::string& inputFilename)
{
    fs::path outpath(gOptions.outputDirectory);
    std::string lastDirName = inputFilename + "-dir";

    render(mesh, *gSkybox1, gOptions.pictureQty,
           outpath / gOptions.skybox1Name / lastDirName);
    render(mesh, *gSkybox2, gOptions.pictureQty,
           outpath / gOptions.skybox2Name / lastDirName);
    render(mesh, *gEmptySkybox, gOptions.pictureQty,
           outpath / gOptions.noSkyboxName / lastDirName);
}

void renderMeshesFromDirectory()
{
    fs::directory_iterator itEnd;
    for (fs::directory_iterator dirIt(gOptions.inputDirectory);
         dirIt != itEnd;
         ++dirIt)
    {
        boost::scoped_ptr<MeshNew> mesh(new MeshNew);
        mesh->loadPLY(dirIt->path().string().c_str());
        renderMesh(*mesh, dirIt->path().filename().string());
    }
}

void onDisplay()
{
    if (gOptions.isCubeModel) {
        boost::scoped_ptr<MeshNew> mesh(new MeshNew);
        mesh->loadCube();
        renderMesh(*mesh, "test-cube");
    } else {
        renderMeshesFromDirectory();
    }

    glutLeaveMainLoop();
}

void initGlut(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_MULTISAMPLE);
    glutInitWindowSize(gOptions.screenWidth, gOptions.screenHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Rendering");
    glutDisplayFunc(onDisplay);
}

void initGL()
{
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

std::string skyboxDirectoryToName(const std::string& dirname)
{
    fs::path path(dirname);
    std::string filename(path.filename().string());
    if ("." == filename)
        filename = path.parent_path().filename().string();

    return filename;
}

bool initOptions(Options& opts, int argc, char** argv)
{
    namespace po = boost::program_options;
    using std::string;

    po::options_description desc("Options");
    desc.add_options()
        ("help", "Print help message")
        ("inputdir",
         po::value<string>(&opts.inputDirectory)->default_value("inputdir"),
         "Input directory")
        ("outputdir",
         po::value<string>(&opts.outputDirectory)->default_value("outputdir"),
         "Output directory")
        ("skybox1",
         po::value<string>(&opts.skybox1Directory)->default_value("skybox1"),
         "First skybox direcotry")
        ("skybox2",
         po::value<string>(&opts.skybox2Directory)->default_value("skybox2"),
         "Second skybox directory")
        ("noskyboxname",
         po::value<string>(&opts.noSkyboxName)->default_value("noskybox"),
         "Output directory name for renders without skybox")
        ("cube",
         "Whether to use test cube model instead of reading from .ply files")
        ("screen-width",
         po::value<int>(&opts.screenWidth)->default_value(800),
         "Screen width")
        ("screen-height",
         po::value<int>(&opts.screenHeight)->default_value(600),
         "Screen height")
        ("picture-qty",
         po::value<int>(&opts.pictureQty)->default_value(10),
         "Quantity of pictures to generate for each model")
        ("fovy-degrees",
         po::value<float>(&opts.fovyDegrees)->default_value(50.0f),
         "Camera's fovy")
        ("initial-angle-degrees",
         po::value<float>(&opts.initialAngleDegrees)->default_value(0.0f),
         "Initial angle in degrees, axis y")
        ("eye-x",
         po::value<float>(&opts.eyeX)->default_value(0.0),
         "Camera's eye.x")
        ("eye-y",
         po::value<float>(&opts.eyeY)->default_value(0.0),
         "Camera's eye.y")
        ("eye-z",
         po::value<float>(&opts.eyeZ)->default_value(2.0),
         "Camera's eye.z")
        ("center-x",
         po::value<float>(&opts.centerX)->default_value(0.0),
         "Camera's center.x")
        ("center-y",
         po::value<float>(&opts.centerY)->default_value(0.3),
         "Camera's center.y")
        ("center-z",
         po::value<float>(&opts.centerZ)->default_value(0.0),
         "Camera's center.z")
        ;
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

    if (vm.count("help")) {
        std::cout << desc << '\n';
        return false;
    }

    po::notify(vm);
    opts.isCubeModel = vm.count("cube");
    opts.skybox1Name = skyboxDirectoryToName(opts.skybox1Directory);
    opts.skybox2Name = skyboxDirectoryToName(opts.skybox2Directory);

    return true;
}

int main(int argc, char** argv)
{
    if (!initOptions(gOptions, argc, argv))
        return EXIT_FAILURE;

    initGlut(argc, argv);
    initGL();

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        std::cerr << "Error: '" << glewGetErrorString(res) << "'\n";
        return EXIT_FAILURE;
    }

    gSkybox1.reset(new Skybox);
    gSkybox2.reset(new Skybox);
    gEmptySkybox.reset(new EmptySkybox);

    gViewParameters.eye = glm::vec3(
            gOptions.eyeX,
            gOptions.eyeY,
            gOptions.eyeZ);
    gViewParameters.center = glm::vec3(
            gOptions.centerX,
            gOptions.centerY,
            gOptions.centerZ);
    gViewParameters.up = glm::vec3(0.0, 1.0, 0.0);

    gProjectionParameters.fovy = glm::radians(gOptions.fovyDegrees);
    gProjectionParameters.aspect =
        1.0f*gOptions.screenWidth/gOptions.screenHeight;
    gProjectionParameters.zNear = 0.1f;
    gProjectionParameters.zFar = 50.0f;

    gSkybox1->load(gOptions.skybox1Directory);
    gSkybox2->load(gOptions.skybox2Directory);
    gEmptySkybox->load("");

    fs::path outDir(gOptions.outputDirectory);
    fs::create_directory(outDir);
    fs::create_directory(outDir / gOptions.skybox1Name);
    fs::create_directory(outDir / gOptions.skybox2Name);
    fs::create_directory(outDir / gOptions.noSkyboxName);

    glutMainLoop();

    return EXIT_SUCCESS;
}
