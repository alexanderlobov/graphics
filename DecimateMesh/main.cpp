#include <iostream>
#include <pcl/io/ply_io.h>
#include <pcl/PolygonMesh.h>
#include <pcl/surface/vtk_smoothing/vtk_mesh_quadric_decimation.h>

void PrintHelp()
{
    std::cerr << "decimate-mesh <input-file> <output-file>\n";
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        PrintHelp();
        return EXIT_FAILURE;
    }
    std::string inputFileName = argv[1];
    std::string outputFileName = argv[2];

    pcl::PolygonMesh::Ptr pInputMesh(new pcl::PolygonMesh);
    pcl::io::loadPLYFile(inputFileName, *pInputMesh);
    std::cout << "size of polygons = "
              << pInputMesh->polygons.size() << '\n';

    pcl::MeshQuadricDecimationVTK decimation;
    decimation.setInputMesh(pInputMesh);
    decimation.setTargetReductionFactor(0.9);

    pcl::PolygonMesh outputMesh;
    decimation.process(outputMesh);

    std::cout << "size of polygons after decimation = "
              << outputMesh.polygons.size() << '\n';

    pcl::io::savePLYFileBinary(outputFileName, outputMesh);

    return 0;
}
