//
//  RasterCoordinates.h
//  VaryingFloodDepth
//
//  Created by a1091793 on 25/09/2015.
//  Copyright © 2015 University of Adelaide. All rights reserved.
//

#ifndef RasterCoordinates_h
#define RasterCoordinates_h

#include <cmath>
#include "Types.h"

Position
getLocationCoordinates(RasterCoordinates location, GeoTransform transform);

RasterCoordinates
getRasterCoordinates(Position location, GeoTransform transform);


#endif /* RasterCoordinates_h */
