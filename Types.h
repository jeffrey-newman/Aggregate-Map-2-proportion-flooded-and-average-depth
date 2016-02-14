//
//  Types.h
//  move_creek
//
//  Created by a1091793 on 23/01/2015.
//  Copyright (c) 2015 University of Adelaide. All rights reserved.
//

#ifndef Types_h
#define Types_h

#include <vector>
#include <tuple>
#include <boost/shared_ptr.hpp>
#include <boost/graph/adjacency_list.hpp>
#include "Map_Matrix.h"

template <typename DataType>
using Map_Matrix_SPtr = boost::shared_ptr<Map_Matrix<DataType> >;

typedef Map_Matrix<double> Map_Double;
typedef boost::shared_ptr<Map_Double> Map_Double_SPtr;

typedef Map_Matrix<int32_t> Map_Int;
typedef boost::shared_ptr<Map_Int> Map_Int_SPtr;

typedef Map_Matrix<bool> Map_Bool;
typedef boost::shared_ptr<Map_Bool> Map_Bool_SPtr;

typedef std::pair<int, int> RasterCoordinates;
typedef std::pair<double, double> Position;
typedef std::vector<RasterCoordinates> BlockSet;
typedef std::shared_ptr<BlockSet> BlockSetSPtr;

struct GeoTransform {
	double x_origin;
	double pixel_width;
	double x_line_space;
	double y_origin;
	double pixel_height;
	double y_line_space;

	GeoTransform()
		: x_origin(0), pixel_width(0), x_line_space(0), y_origin(0), pixel_height(0), y_line_space(0)
	{

	}
};


#endif
