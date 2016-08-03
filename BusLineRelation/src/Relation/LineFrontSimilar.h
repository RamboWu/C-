#ifndef LINE_FRONT_SIMILAR_H
#define LINE_FRONT_SIMILAR_H

#include <map>
#include "../Bus/BusLineManager.h"
#include "LineSimilar.h"

class LineFrontSimilar {

	LineReport* line_report;

public:

	std::map<SimilarMeta, double, SimilarCompare> similar_rate;

	void CalcSimilar(BusLineManager* busline_manager);

protected:
	void Calc(BusLineManager* busline_manager);
	void Report(BusLineManager* busline_manager);

	std::map<int, std::set<int>> similar;

};

#endif