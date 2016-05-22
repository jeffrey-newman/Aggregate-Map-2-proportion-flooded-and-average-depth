//
//  AggregateMap.h
//  aggregate-map-2v2
//
//  Created by a1091793 on 21/05/2016.
//
//

#ifndef AggregateMap_h
#define AggregateMap_h

#include <blink/raster/gdal_raster.h>


typedef blink::raster::gdal_raster<double> DoubleRaster;
typedef blink::raster::gdal_raster<int> IntRaster;

void
aggregateMaps(DoubleRaster & map, IntRaster & template_m, DoubleRaster & output_map_depth, DoubleRaster & output_map_proportion, bool ignore_na = true, bool do_max = false, bool majority = false);

#endif /* AggregateMap_h */
