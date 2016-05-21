//
//  Main.cpp
//  aggregate-map-2v2
//
//  Created by a1091793 on 21/05/2016.
//
//


#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <list>

//#include <GDAL/gdal.h>

#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/foreach.hpp>
#include <boost/progress.hpp>

//#include <Eigen/Dense>

//#include "Types.h"
//#include "Neighbourhood.h"
//#include "RasterCoordinates.h"

#include <blink/raster/utility.h>

#include "AggregateMap.h"

int main(int argc, char **argv)
{
    
    
    /**********************************/
    /*        Program options         */
    /**********************************/
    // Need to specify elevation grid
    // Need to specify channel
    std::string map_file;
    std::string template_file;
    std::string output_file_depth;
    std::string output_file_proportion;
    
    bool ignore_na = false;
    bool do_max = false;
    bool majority = false;
    //
    
    namespace prog_opt = boost::program_options;
    namespace fs = boost::filesystem;
    namespace raster_util = blink::raster;
    
    prog_opt::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("map,m", prog_opt::value<std::string>(&map_file), "path of the gdal capatible raster for aggregating")
    ("template,t", prog_opt::value<std::string>(&template_file), "path of the gdal capatible raster which works as a template (output raster will have same extent and cell size as template)")
    ("depth-out,d", prog_opt::value<std::string>(&output_file_depth)->default_value("aggregated-depth.tif"), "path where the output gtif raster of depth is saved")
    ("proportion-out,p", prog_opt::value<std::string>(&output_file_proportion)->default_value("aggregated-proportion.tif"), "path where the output gtif raster of proportion flooded is saved")
    ("maximum,x", "use maximum statistic, rather than average")
    ("majority,a", "only assign value if majority of cells are not noValue")
    ("ignore-na,i", "ignore pixels with nodata when taking statistic");
    prog_opt::variables_map vm;
    prog_opt::store(prog_opt::parse_command_line(argc, argv, desc), vm);
    prog_opt::notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc << "\n";
        return 1;
    }
    if (vm.count("maximum"))
    {
        do_max = true;
    }
    if (vm.count("ignore-na"))
    {
        ignore_na = true;
    }
    if (vm.count("majority"))
    {
        majority = true;
    }
    
    fs::path map_path(map_file);
    fs::path template_path(template_file);
    fs::path output_path_depth(output_file_depth);
    fs::path output_path_proportion(output_file_proportion);
    
    
    // Check file exists
    if (!fs::exists(map_path))
    {
        std::stringstream ss;
        ss << map_path << " does not exist";
        throw std::runtime_error(ss.str());
        return (EXIT_FAILURE);
    }
    
    if (!fs::exists(template_path))
    {
        std::stringstream ss;
        ss << template_path << " does not exist";
        throw std::runtime_error(ss.str());
        return (EXIT_FAILURE);
    }
    
    
    /**********************************/
    /*         Read in maps           */
    /**********************************/
    std::cout << "\n\n*************************************\n";
    std::cout <<     "*             Read in maps          *\n";
    std::cout <<     "*************************************" << std::endl;
    auto template_m = raster_util::open_gdal_raster<double>(template_path, GA_ReadOnly);
    
    auto map = raster_util::open_gdal_raster<double>(map_path, GA_ReadOnly);
    
    
    /********************************************/
    /*       Assign memory for output map       */
    /********************************************/
    std::cout << "\n\n**************************************\n";
    std::cout <<     "*    Assign memory for output maps   *\n";
    std::cout <<     "**************************************" << std::endl;
    auto output_map_depth = raster_util::create_gdal_raster_from_model<double>(output_path_depth, template_m);
    auto output_map_proportion = raster_util::create_gdal_raster_from_model<double>(output_path_proportion, template_m);
    
    
    aggregateMaps(map, template_m, output_map_depth, output_map_proportion, ignore_na, do_max, majority);
    
    return (EXIT_SUCCESS);
}

