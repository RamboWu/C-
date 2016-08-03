#ifndef _ANALYSE_MILEAGES_TOOLS_H_
#define _ANALYSE_MILEAGES_TOOLS_H_

#include "../Bus/Bus.h"
#include "../Bus/BusLineIndexInfo.h"

class MileagesTools {

public:

	static double GetPointMilesOnRoutes(BusLine* line, int dir, GPSPoint& point, std::vector<BusLineInPlaneUnit*>& routes_set) {
		double min_dis = 100000000;
		double result = 0;

		for (int t = 0; t < routes_set.size(); t++) {
			Segment segment = Segment(line->routes[dir][routes_set[t]->route_serial - 1], line->routes[dir][routes_set[t]->route_serial]);
			double gpsAP = segment.start.GPSDistance(point);
			double dis = segment.NearestDistanceWithMetre(point, gpsAP);
			double len = segment.PointShadowWithMetre(point, gpsAP) + line->sum_routes[dir][routes_set[t]->route_serial - 1];
			if (dis < min_dis) {
				min_dis = dis;
				result = len;
			}
		}

		return result;
	}

	static double GetPointMilesOnRoutes(BusLine* line, int dir, GPSPoint& point, double start) {
		double min_dis = 100000000;
		double result = 0;

		for (int t = 0; t < line->routes[dir].size() - 1; t++) {
			Segment segment = Segment(line->routes[dir][t], line->routes[dir][t + 1]);

			double gpsAP = segment.start.GPSDistance(point);

			double dis = segment.NearestDistanceWithMetre(point, gpsAP);
			double len = line->sum_routes[dir][t] + segment.PointShadowWithMetre(point, gpsAP);
			if (len >= start - 20 && dis <= 50) {
				min_dis = dis;
				result = len;
				break;
			}

			if (dis < min_dis - 20) {
				min_dis = dis;
				result = len;
			}
		}

		return result;
	}

	static double GetPointMilesOnRoutes(BusLine* line, int dir, GPSPoint& point, double start, int dis_error) {
		double min_dis = 100000000;
		double result = 0;

		for (int t = 0; t < line->routes[dir].size() - 1; t++) {
			Segment segment = Segment(line->routes[dir][t], line->routes[dir][t + 1]);

			double gpsAP = segment.start.GPSDistance(point);

			double dis = segment.NearestDistanceWithMetre(point, gpsAP);
			double len = line->sum_routes[dir][t] + segment.PointShadowWithMetre(point, gpsAP);
			if (len >= start - 20 && dis <= dis_error) {
				min_dis = dis;
				result = len;
				break;
			}

			if (dis < min_dis) {
				min_dis = dis;
				result = len;
			}
		}

		return result;
	}

	static double GetPointMilesOnRoutes(BusLine* line, int dir, GPSPoint& point, int index_error, double start, const std::vector<BusLineInPlaneUnit*>& routes_set) {
		double min_dis = 100000000;
		double result = 0;

		bool flag = false;
		for (int t = 0; t < routes_set.size(); t++) {
			Segment segment = Segment(line->routes[dir][routes_set[t]->route_serial - 1], line->routes[dir][routes_set[t]->route_serial]);
			double gpsAP = segment.start.GPSDistance(point);

			double dis = segment.NearestDistanceWithMetre(point, gpsAP);
			double len = segment.PointShadowWithMetre(point, gpsAP);
			if (routes_set[t]->route_serial > 2) len += line->sum_routes[dir][routes_set[t]->route_serial - 1];
			if (len >= start - 20 && dis <= index_error) {
				if (dis > 0 && (dis + 20 < min_dis || !flag)) {
					min_dis = dis;
					result = len;
					flag = true;
				}
			}
			if (dis < min_dis && !flag) {
				min_dis = dis;
				result = len;
			}
		}

		return result;
	}

	static double GetPointToRoutesMinDistance(BusLine* line, int dir, GPSPoint& point, double start) {
		double min_dis = 100000000;
		double result = 0;

		for (int t = 0; t < line->routes[dir].size() - 1; t++) {
			Segment segment = Segment(line->routes[dir][t], line->routes[dir][t + 1]);

			double gpsAP = segment.start.GPSDistance(point);

			double dis = segment.NearestDistanceWithMetre(point, gpsAP);
			double len = line->sum_routes[dir][t] + segment.PointShadowWithMetre(point, gpsAP);
			if (len >= start - 20 && dis<min_dis) {
				min_dis = dis;
				result = len;
			}
		}

		return min_dis;
	}

};
#endif