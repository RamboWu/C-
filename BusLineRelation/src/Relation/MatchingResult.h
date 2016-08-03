#ifndef MATCHING_RESULT_H
#define MATCHING_RESULT_H

#include <map>

class MatchingResult {
public:
	std::map<int, int> line_to_bus_num;
	std::map<int, int> bus_to_line_num;
	std::map<int, int> station_to_bus_num;
	std::map<int, double> line_to_bus;
	std::map<int, double> bus_to_line;
	std::map<int, double> station_to_bus;
};

#endif