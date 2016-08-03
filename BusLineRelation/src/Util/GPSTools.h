#ifndef GPS_TOOLS_H
#define GPS_TOOLS_H

#include <math.h>
#include "../Index/PlaneIndex.h"
#include "Configuration.h"

class GPSTools {

	static double PI;
	static double D2R;
public:
	static double GeoDistance(GPSPoint* point1, GPSPoint* point2) {
		double r = 6370669.81;
		double lat1 = point1->lat*D2R;
		double lon1 = point1->lon*D2R;
		double lat2 = point2->lat*D2R;
		double lon2 = point2->lon*D2R;
		double d1 = abs(lat1 - lat2);
		double d2 = abs(lon1 - lon2);
		double p = pow(sin(d1 / 2.0), 2) + cos(lat1)
			* cos(lat2) * pow(sin(d2 / 2), 2);
		double dis = r * 2 * asin(sqrt(p));
		return dis;
	}

	static bool GPSEqual(GPSPoint* point1, GPSPoint* point2) {
		double dis = GeoDistance(point1, point2);

		if (dis <= Configuration::gps_equal_error)
			return true;
		else
			return false;
	}

	static bool OutOfChina(double lon, double lat) {
		if (lon > 73.666 && lat > 4 && lat < 53.5 && lon < 135.834)
			return false;
		else
			return true;
	}
};

double GPSTools::PI = 3.1415926;
double GPSTools::D2R = PI / 180.0;

#endif