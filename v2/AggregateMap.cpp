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
aggregateMaps(DoubleRaster & map, IntRaster & template_m, DoubleRaster & output_map_depth, DoubleRaster & output_map_proportion, bool ignore_na, bool do_max, bool majority)
{
//	output_map_depth->SetNoDataValue(out_map_no_data_val);
//    output_map_proportion->SetNoDataValue(out_map_no_data_val);

    namespace raster_util = blink::raster;
    int template_rows = template_m.nRows();
    int template_cols = template_m.nCols();
    double template_no_data =  const_cast<GDALRasterBand *>(template_m.get_gdal_band())->GetNoDataValue();
    
    double gt_data[6];
    //    double* temp_geotransform = gt_data;
    CPLErr error_status
    = const_cast<GDALDataset*>(template_m.get_gdal_dataset())->GetGeoTransform(gt_data);
    GeoTransform template_transform(gt_data);
    
    error_status
    = const_cast<GDALDataset*>(map.get_gdal_dataset())->GetGeoTransform(gt_data);
    GeoTransform map_transform(gt_data);
    double no_data_val = const_cast<GDALRasterBand *>(map.get_gdal_band())->GetNoDataValue();
    
    
    //Identify guage nodes from a guage node file
    /********************************************/
    /*       Do the aggregation     */
    /********************************************/
    std::cout << "\n\n*************************************\n";
    std::cout <<     "*       Aggregating the maps        *\n";
    std::cout <<     "*************************************" << std::endl;

    double out_map_no_data_val = 0;
    const_cast<GDALRasterBand *>(output_map_depth.get_gdal_band())->SetNoDataValue(out_map_no_data_val);
    const_cast<GDALRasterBand *>(output_map_proportion.get_gdal_band())->SetNoDataValue(out_map_no_data_val);


	for (unsigned int i = 0; i < template_rows; ++i)
	{
		for (unsigned int j = 0; j < template_cols; ++j)
		{
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
				double sum = 0;
				double max = std::numeric_limits<double>::min();
				int count = 0;
                int proportion_count = 0;
                int num_cells = 0;
				int no_data_count = 0;
				BOOST_FOREACH(RasterCoordinates & coord, *block)
				{
					double val = map.get(raster_util::coordinate_2d(coord.first, coord.second));
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

	