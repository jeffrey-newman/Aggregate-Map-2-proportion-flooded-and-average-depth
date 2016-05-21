
#include "RasterCoordinates.h"


Position
getLocationCoordinates(RasterCoordinates location, GeoTransform transform)
{
	Position coordinates; //(row, col)
	coordinates.first = location.first * transform.y_line_space + transform.y_origin;
	coordinates.second = location.second * transform.pixel_width + transform.x_origin;
	return (coordinates);
}

RasterCoordinates
getRasterCoordinates(Position location, GeoTransform transform)
{
	RasterCoordinates coordinates; //(row, col)
	coordinates.first = std::floor((location.first - transform.y_origin) / transform.y_line_space);
	coordinates.second = std::floor((location.second - transform.x_origin) / transform.pixel_width);
	return (coordinates);
}