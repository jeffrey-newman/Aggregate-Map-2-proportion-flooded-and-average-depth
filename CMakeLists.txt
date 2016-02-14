CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

PROJECT(aggregate-map-2)

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_STATIC_RUNTIME ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem system program_options)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

FIND_PACKAGE(gdal)
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR})

SET (AGGREGATE_HEADERS
		Map_Matrix.h
		Neighbourhood.h
		Print.h
		RasterCoordinates.h
		ReadInMap.h
		Types.h

    )

SET (AGGREGATE_SOURCES 
        AggregateMap.cpp
		Map_Matrix_Impl.cpp
		Map_Matrix.cpp
		Neighbourhood_Impl.cpp
		Neighbourhood.cpp
		Print_Impl.cpp
		Print.cpp
		RasterCoordinates.cpp
		ReadInMap_Impl.cpp
		ReadInMap.cpp
        )
        
ADD_EXECUTABLE(aggregate-map-2 ${AGGREGATE_SOURCES} ${AGGREGATE_HEADERS})
TARGET_LINK_LIBRARIES(aggregate-map-2 "${GDAL_LIBRARY}" "${Boost_LIBRARIES}" )

set_property(TARGET aggregate-map-2 PROPERTY CXX_STANDARD 11)
