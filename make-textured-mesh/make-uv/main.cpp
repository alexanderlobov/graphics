#include <iostream>
#include <OGF/cells/types/cells_library.h>
#include <OGF/cells/map/map.h>
#include <OGF/cells/map/map_attributes.h>
#include <OGF/cells/map/map_editor.h>
#include <OGF/cells/map/geometry.h>
#include <OGF/cells/map_algos/atlas_generator.h>
#include <OGF/cells/map_algos/pm_manager.h>
#include <OGF/image/types/image.h>
#include <OGF/image/types/image_library.h>
#include <OGF/image/algos/rasterizer.h>
#include <OGF/image/algos/morpho_math.h>
#include <OGF/basic/os/file_system.h>
#include <stdlib.h>

namespace OGF {

void generate_atlas(Map* the_map) {
    AtlasGenerator the_generator(the_map) ;
    the_generator.set_unglue_hardedges(false) ;
    the_generator.set_auto_cut(true) ;
    the_generator.set_auto_cut_cylinders(true) ;
    the_generator.set_parameterizer("LSCM") ;
    the_generator.set_max_overlap_ratio(0.0001) ;
    the_generator.set_max_scaling(120.0) ;
    the_generator.set_min_fill_ratio(0.25) ;
    the_generator.set_pack(true) ;
    the_generator.set_splitter("VSASmooth") ;
    the_generator.apply() ;
}

}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input-file> <output-file>\n";
        return -1;
    }

    // if(argc != 3 && argc != 4) {
    //     std::cerr << "usage: " << argv[0] << " <input_file> <output_file> [decim proportion]" << std::endl ;
    //     exit(-1) ;
    // }

    std::string in = argv[1] ;
    std::string out = argv[2] ;
    // std::string image = OGF::FileSystem::dir_name(out) + "/" + OGF::FileSystem::base_name(out) + ".png" ;
    // double proportion = 0.1 ;
    // if(argc >= 4) {
    //     proportion = atof(argv[3]) ;
    // }
    // 

    OGF::Map the_map ;
    std::cerr << "==== Step 1/5 == Loading file: " << in << std::endl ;
    if(!OGF::CellsLibrary::instance()->load_map(in, &the_map)) {
        std::cerr << "Could not open file " << in << std::endl ;
        exit(-1) ;
    } 

    std::cerr << "nb facets: " << the_map.size_of_facets() << " nb vertices:" << the_map.size_of_vertices() << std::endl ;
    std::cerr << std::endl ;

    // the_map.compute_normals() ;

    std::cerr << "==== Step 2/5 == Generating texture atlas" << std::endl ;
    OGF::generate_atlas(&the_map) ;

    // std::cerr << "==== Step 3/5 == Generating normal map: " << image << std::endl ;
    // OGF::generate_normal_map(&the_map, image) ;

    // std::cerr << "==== Step 4/5 == Decimating surface - proportion = " << proportion << std::endl ;
    // OGF::decimate_surface(&the_map, proportion) ;

    std::cerr << "==== Step 5/5 == Saving surface: " << out << std::endl ;
    if(!OGF::CellsLibrary::instance()->save_map(out, &the_map)) {
        std::cerr << "Could not write file " << out << std::endl ;
        exit(-1) ;
    }


    return 0 ;
}
