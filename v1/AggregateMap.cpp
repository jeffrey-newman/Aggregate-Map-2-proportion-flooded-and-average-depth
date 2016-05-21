#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <list>

#include <GDAL/gdal.h>

#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/foreach.hpp>
#include <boost/progress.hpp>

//#include <Eigen/Dense>

#include "Types.h"
#include "ReadInMap.h"
#include "Print.h"
#include "Neighbourhood.h"
#include "RasterCoordinates.h"



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
	std::tuple<Map_Double_SPtr, std::string, GeoTransform> gdal_template = read_in_map<double>(template_path, GDT_Float64, NO_CATEGORISATION);
	Map_Double_SPtr template_m(std::get<0>(gdal_template));
	std::string & template_WKT_projection(std::get<1>(gdal_template));
	GeoTransform & template_transform(std::get<2>(gdal_template));

	int template_rows = template_m->NRows();
	int template_cols = template_m->NCols();
	double template_no_data = template_m->NoDataValue();
	/*template_m.reset(new Map_Double);
	std::get<0>(gdal_template).reset(new Map_Double);*/

	std::tuple<Map_Double_SPtr, std::string, GeoTransform> gdal_map = read_in_map<double>(map_path, GDT_Float64, NO_CATEGORISATION);
	Map_Double_SPtr map(std::get<0>(gdal_map));
	std::string & map_WKT_projection(std::get<1>(gdal_map));
	GeoTransform & map_transform(std::get<2>(gdal_map));
    
	
	/********************************************/
	/*       Assign memory for output map       */
	/********************************************/
	std::cout << "\n\n*************************************\n";
	std::cout <<     "*    Assign memory for output map   *\n";
	std::cout <<     "*************************************" << std::endl;
	double out_map_no_data_val = 0;
	Map_Double_SPtr output_map_depth(new Map_Double(template_rows, template_cols, out_map_no_data_val));
    Map_Double_SPtr output_map_proportion(new Map_Double(template_rows, template_cols, out_map_no_data_val));
	output_map_depth->SetNoDataValue(out_map_no_data_val);
    output_map_proportion->SetNoDataValue(out_map_no_data_val);


    //Identify guage nodes from a guage node file
    /********************************************/
    /*       Do the aggregation     */
    /********************************************/
    std::cout << "\n\n*************************************\n";
    std::cout <<     "*       Aggregating the maps        *\n";
    std::cout <<     "*************************************" << std::endl;
	
	double no_data_val = map->NoDataValue();

	for (unsigned int i = 0; i < template_rows; ++i)
	{
		for (unsigned int j = 0; j < template_cols; ++j)
		{
			if (template_m->Get(i, j) != template_no_data)
			{
				RasterCoordinates top_left_c = std::make_pair(i, j);
				RasterCoordinates bottom_left_c = std::make_pair(i + 1, j);
				RasterCoordinates top_right_c = std::make_pair(i, j + 1);

				Position top_left_p = getLocationCoordinates(top_left_c, template_transform);
				Position bottom_left_p = getLocationCoordinates(bottom_left_c, template_transform);
				Position top_right_p = getLocationCoordinates(top_right_c, template_transform);

				BlockSetSPtr block = getBlock(map, map_transform, top_left_p, bottom_left_p, top_right_p);

				//Do the aggregation by taking the mean. Treat NoData cells as 0.
				double sum = 0;
				double max = std::numeric_limits<double>::min();
				int count = 0;
                int proportion_count = 0;
                int num_cells = 0;
				int no_data_count = 0;
				BOOST_FOREACH(RasterCoordinates & coord, *block)
				{
					double val = map->Get(coord.first, coord.second);
                    ++num_cells;
					if (val == no_data_val)
					{
						++no_data_count;
						if (ignore_na)
						{
							--count;
						}
						else
						{
							sum += 0;
							if (val > max) max = val;
						}
					}
					else
					{
                        ++proportion_count;
						sum += val;
						if (val > max) max = val;
					}
					
					++count;
				}
				double mean = sum / count;
				if (do_max) output_map_depth->Get(i, j) = max;
				else output_map_depth->Get(i, j) = mean;
                output_map_proportion->Get(i, j) = proportion_count / double(num_cells);
				if (no_data_count != 0)
				{
					if (((count / no_data_count) < 2) && majority)
					{
						output_map_depth->Get(i, j) = out_map_no_data_val;
					}
				}
                
			}
			else
			{
				output_map_depth->Get(i, j) = out_map_no_data_val;
			}
		}
	}

    
    std::cout << "\n\n*************************************\n";
    std::cout << "*         Saving output file        *\n";
    std::cout << "*************************************" << std::endl;
    std::string driverName = "GTiff";
	write_map(output_path_depth, GDT_Float64, output_map_depth, template_WKT_projection, template_transform, driverName);
    write_map(output_path_proportion, GDT_Float64, output_map_proportion, template_WKT_projection, template_transform, driverName);
    
    return (EXIT_SUCCESS);
}

	