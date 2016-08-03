#ifndef LINE_FRONT_SIMILAR_H
#define LINE_FRONT_SIMILAR_H

#include <map>
#include "LineSimilar.h"

class BusLineManager;

class LineFrontSimilar {

	static const int LINE_FRONT_LENGTH;
	LineReport* line_report;

public:

	std::map<SimilarMeta, double, SimilarCompare> similar_rate;

	void CalcSimilar(BusLineManager* busline_manager);

protected:
	set<GPSPoint, GPSPointCompare> getLineFrontPointsInIndex(BusLineManager* busline_manager, int dir_serial);

	int getIntersectionCount(set<GPSPoint, GPSPointCompare>& a, set<GPSPoint, GPSPointCompare>& b);
	void Calc(BusLineManager* busline_manager);
	void Report(BusLineManager* busline_manager);

	std::map<int, std::set<int>> similar;

};

#endif