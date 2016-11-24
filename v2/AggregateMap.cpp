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

#include "Types.h"
#include "Neighbourhood.h"
#include "RasterCoordinates.h"

#include <blink/raster/utility.h>

#include "AggregateMap.h"

void
aggregateMaps(FloatRaster & map, FloatRaster & template_m, FloatRaster & output_map_depth, FloatRaster & output_map_proportion, bool ignore_na, bool do_max, bool majority)
{
//	output_map_depth->SetNoDataValue(out_map_no_data_val);
//    output_map_proportion->SetNoDataValue(out_map_no_data_val);

    namespace raster_util = blink::raster;
    int template_rows = template_m.nRows();
    int template_cols = template_m.nCols();
    float template_no_data =  const_cast<GDALRasterBand *>(template_m.get_gdal_band())->GetNoDataValue();
    
    double gt_data[6];
    //    double* temp_geotransform = gt_data;
    CPLErr error_status
    = const_cast<GDALDataset*>(template_m.get_gdal_dataset())->GetGeoTransform(gt_data);
    GeoTransform template_transform(gt_data);
    
    error_status
    = const_cast<GDALDataset*>(map.get_gdal_dataset())->GetGeoTransform(gt_data);
    GeoTransform map_transform(gt_data);
    float no_data_val = const_cast<GDALRasterBand *>(map.get_gdal_band())->GetNoDataValue();
    
    
    //Identify guage nodes from a guage node file
    /********************************************/
    /*       Do the aggregation     */
    /********************************************/
    std::cout << "\n\n*************************************\n";
    std::cout <<     "*       Aggregating the maps        *\n";
    std::cout <<     "*************************************" << std::endl;

    float out_map_no_data_val = 0;
    const_cast<GDALRasterBand *>(output_map_depth.get_gdal_band())->SetNoDataValue(out_map_no_data_val);
    const_cast<GDALRasterBand *>(output_map_proportion.get_gdal_band())->SetNoDataValue(out_map_no_data_val);

    boost::progress_display show_progress1((template_rows*template_cols));

	for (unsigned int i = 0; i < template_rows; ++i)
	{
		for (unsigned int j = 0; j < template_cols; ++j)
		{
            ++show_progress1;
            if (template_m.get(raster_util::coordinate_2d(i, j)) != template_no_data)
			{
				RasterCoordinates top_left_c = std::make_pair(i, j);
				RasterCoordinates bottom_left_c = std::make_pair(i + 1, j);
				RasterCoordinates top_right_c = std::make_pair(i, j + 1);

				Position top_left_p = getLocationCoordinates(top_left_c, template_transform);
				Position bottom_left_p = getLocationCoordinates(bottom_left_c, template_transform);
				Position top_right_p = getLocationCoordinates(top_right_c, template_transform);

				BlockSetSPtr block = getBlock(map, map_transform, top_left_p, bottom_left_p, top_right_p);

				//Do the aggregation by taking the mean. Treat NoData cells as 0.
				float sum = 0;
				float max = std::numeric_limits<float>::min();
				int count = 0;
                int proportion_count = 0;
                int num_cells = 0;
				int no_data_count = 0;
				BOOST_FOREACH(RasterCoordinates & coord, *block)
				{
					float val = map.get(raster_util::coordinate_2d(coord.first, coord.second));
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
				float mean = sum / count;
				if (do_max) output_map_depth.put(raster_util::coordinate_2d(i, j), max);
				else output_map_depth.put(raster_util::coordinate_2d(i, j), mean);
                output_map_proportion.put(raster_util::coordinate_2d(i, j), proportion_count / double(num_cells));
				if (no_data_count != 0)
				{
					if (((count / no_data_count) < 2) && majority)
					{
						output_map_depth.put(raster_util::coordinate_2d(i, j), out_map_no_data_val);
					}
				}
                
			}
			else
			{
				output_map_depth.put(raster_util::coordinate_2d(i, j), out_map_no_data_val);
                output_map_proportion.put(raster_util::coordinate_2d(i, j), out_map_no_data_val);
			}
		}
	}

    
//    std::cout << "\n\n*************************************\n";
//    std::cout << "*         Saving output file        *\n";
//    std::cout << "*************************************" << std::endl;
//    std::string driverName = "GTiff";
//	write_map(output_path_depth, GDT_Float64, output_map_depth, template_WKT_projection, template_transform, driverName);
//    write_map(output_path_proportion, GDT_Float64, output_map_proportion, template_WKT_projection, template_transform, driverName);
    

}


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
    
    boost::filesystem::path map_path(map_file);
    boost::filesystem::path template_path(template_file);
    boost::filesystem::path output_path_depth(output_file_depth);
    boost::filesystem::path output_path_proportion(output_file_proportion);
    
    
    // Check file exists
    if (!boost::filesystem::exists(map_path))
    {
        std::stringstream ss;
        ss << map_path << " does not exist";
        throw std::runtime_error(ss.str());
        return (EXIT_FAILURE);
    }
    
    if (!boost::filesystem::exists(template_path))
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

    boost::shared_ptr<blink::raster::gdal_raster<float> > map
            = blink::raster::open_gdal_rasterSPtr<float>(map_path, GA_ReadOnly);
    boost::shared_ptr<blink::raster::gdal_raster<float> > template_m
        = blink::raster::open_gdal_rasterSPtr<float>(template_path, GA_ReadOnly);
    boost::shared_ptr<blink::raster::gdal_raster<float> > output_map_depth
            = blink::raster::create_gdal_rasterSPtr_from_model<float>(output_path_depth, *template_m, GDT_Float32);
    boost::shared_ptr<blink::raster::gdal_raster<float> > output_map_proportion
            = blink::raster::create_gdal_rasterSPtr_from_model<float>(output_path_proportion, *template_m, GDT_Float32);
    
    
    aggregateMaps(*map, *template_m, *output_map_depth, *output_map_proportion, ignore_na, do_max, majority);
    
    
    
    return (EXIT_SUCCESS);
}

