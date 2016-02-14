//
//  Neighbourhood_Impl.cpp
//  move_creek
//
//  Created by Jeffrey Newman on 28/01/2015.
//  Copyright (c) 2015 University of Adelaide. All rights reserved.
//

#include <stdio.h>
#include "Types.h"
#include <GDAL/gdal.h>
#include "Neighbourhood.cpp"

//template
//boost::shared_ptr<Set> get_neighbourhood(Map_Double_SPtr map, unsigned long i, unsigned long j, unsigned long size);
//
//template
//boost::shared_ptr<Set> get_neighbourhood(Map_Int_SPtr map, unsigned long i, unsigned long j, unsigned long size);
//
//template
//boost::shared_ptr<Set> find_adjacents(Map_Int_SPtr map, unsigned long i, unsigned long j, unsigned long size);
//
//template
//boost::shared_ptr<std::vector<std::pair<int, int> > > find_immediate_adjacents(Map_Double_SPtr map, unsigned long i, unsigned long j);

template
BlockSetSPtr getBlock(Map_Double_SPtr map, GeoTransform & transform, Position top_left, Position bottom_left, Position top_right);
