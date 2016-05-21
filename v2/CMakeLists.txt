CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

PROJECT(aggregate-map-2v2)

# SET(Boost_USE_STATIC_LIBS ON)
# SET(Boost_USE_STATIC_RUNTIME ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem system program_options)

set(AHHZ_RASTER_UTIL "Not Specified" CACHE PATH "Path to Alex Hagen-Zanker's Raster Utility Include direction (select above blink)")
set(AHHZ_RASTER_ITERATOR "Not Specified" CACHE PATH "Path to Alex Hagen-Zanker's Raster Iterator Include direction (select folder above blink)")

include_directories(${GDAL_INCLUDE_DIR} )
include_directories(${Boost_INCLUDE_DIRS} )
include_directories(${AHHZ_RASTER_UTIL} )
include_directories(${AHHZ_RASTER_ITERATOR} )

FIND_PACKAGE(gdal)
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR})

SET (AGGREGATE_HEADERS

		Neighbourhood.h
		RasterCoordinates.h
		Types.h

    )

SET (AGGREGATE_SOURCES 
        AggregateMap.cpp

		Neighbourhood_Impl.cpp
		Neighbourhood.cpp

		RasterCoordinates.cpp
		
        )
        
file(GLOB_RECURSE AHHZ_Utils
   "${AHHZ_RASTER_UTIL}/*.h"
   "${AHHZ_RASTER_UTIL}/*.hpp"
   "${AHHZ_RASTER_UTIL}/*.cpp"
   "${AHHZ_RASTER_ITERATOR}/*.h"
   "${AHHZ_RASTER_ITERATOR}/*.hpp"
   "${AHHZ_RASTER_ITERATOR}/*.cpp"
)
        
ADD_EXECUTABLE(aggregate-map-2 ${AGGREGATE_SOURCES} ${AGGREGATE_HEADERS} ${AHHZ_Utils})
TARGET_LINK_LIBRARIES(aggregate-map-2 "${GDAL_LIBRARY}" "${Boost_LIBRARIES}" )

set_property(TARGET aggregate-map-2 PROPERTY CXX_STANDARD 11)
