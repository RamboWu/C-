#include "LineFrontSimilar.h"
#include "../Bus/BusLineManager.h"


const int LineFrontSimilar::LINE_FRONT_LENGTH = 1000;

void LineFrontSimilar::CalcSimilar(BusLineManager* busline_manager) {

	Calc(busline_manager);
	Report(busline_manager);

}

void LineFrontSimilar::Calc(BusLineManager* busline_manager) {
	similar.clear();
	//求出similar数组,与线first相似的有set个
	for (int i = 0; i < busline_manager->line_set.size(); i++) {
		BusLine* line = busline_manager->line_set[i];

		std::map<int, int> temp_similar[2];

		for (int k = 0; k < line->dir_num; k++) {

			std::map<int, set<GPSPoint, GPSPointCompare>> line_similar;
			set<GPSPoint, GPSPointCompare> line_front_points = getLineFrontPointsInIndex(busline_manager, line->serial * 2 + k);
			double now_front_total_length = 0;
			//求得与当前dir_serial有相交的所有line
			for (int j = 0; j < (int)line->routes[k].size() - 1; j++) {

				Segment segment = Segment(line->routes[k][j], line->routes[k][j + 1]);
				std::list<std::pair<GPSPoint*, BusLineUnit*>*>* unit_list = busline_manager->line_index->GetSegment(segment, 1);

				for (auto iter = unit_list->begin(); iter != unit_list->end(); iter++) {
					GPSPoint* point = (*iter)->first;

					std::list<std::pair<GPSPoint*, BusLineUnit*>*>* unit = busline_manager->line_index->GetPoint(*point, 2);
					for (auto iter1 = unit->begin(); iter1 != unit->end(); iter1++) {
						
						for (auto iter2 = (*iter1)->second->line_dir_sets.begin(); iter2 != (*iter1)->second->line_dir_sets.end(); iter2++) {
							if (line_similar.find(iter2->first) == line_similar.end()) {
								set<GPSPoint, GPSPointCompare> temp;
								line_similar[iter2->first] = temp;
							}
							line_similar[iter2->first].insert(*((*iter1)->first));
							
						}
					}
				}

				delete unit_list;

			}

			for (auto iter = line_similar.begin(); iter != line_similar.end(); iter++) {
				int dir = iter->first % 2;
				int line_serial = iter->first >> 1;
				if (line_serial == line->serial) continue;
				int size = iter->second.size();

				if (size >= line_front_points.size() * 0.80) {
					//取这一条可能的线的 头部的空间索引
					set<GPSPoint, GPSPointCompare> target_line_front_point = getLineFrontPointsInIndex(busline_manager, iter->first);
					int inter_count = getIntersectionCount(line_front_points, target_line_front_point);
					
					if ((inter_count >= line_front_points.size()*0.85) && isStartNear(iter->first, line->serial * 2 + k, busline_manager)) {
						if (similar.find(line->serial) == similar.end()) {
							similar[line->serial] = std::set<int>();
						}
						similar[line->serial].insert(line_serial);
					}
					
				}
			}
		}
		
	}

}

void LineFrontSimilar::Report(BusLineManager* busline_manager) {
	//输出父子关系，Modified By WuPeiFeng
	FILE* temp_out = fopen(Configuration::line_front_similar_report.c_str(), "w");
	for (auto iter = similar.begin(); iter != similar.end(); iter++) {
		int line_id = iter->first;
		BusLine* line = busline_manager->GetLine(line_id);
		fprintf(temp_out, "%s:", line->line_id.c_str());
		bool is_first = true;
		for (auto iter1 = iter->second.begin(); iter1 != iter->second.end(); iter1++) {
			if (!is_first)
				fprintf(temp_out, ",");
			is_first = false;
			BusLine* line1 = busline_manager->GetLine(*iter1);
			fprintf(temp_out, "%s", line1->line_id.c_str());
		}
		fprintf(temp_out, "\n");
	}
	fclose(temp_out);
}

set<GPSPoint, GPSPointCompare> LineFrontSimilar::getLineFrontPointsInIndex(BusLineManager* busline_manager, int dir_serial) {
	int dir = dir_serial % 2;
	int serial = dir_serial >> 1;

	BusLine* line = busline_manager->GetLine(serial);

	set<GPSPoint, GPSPointCompare> line_front_points;
	double now_front_total_length = 0;
	//求得与当前dir_serial有相交的所有line
	for (int j = 0; j < (int)line->routes[dir].size() - 1; j++) {
		double piece_length = line->routes[dir][j].GPSDistance(line->routes[dir][j + 1]);
		double left_length;
		if (now_front_total_length + piece_length > LINE_FRONT_LENGTH) {
			left_length = LINE_FRONT_LENGTH - now_front_total_length;
		}
		else {
			left_length = piece_length;
		}
		now_front_total_length += piece_length;

		Segment segment = Segment(line->routes[dir][j], line->routes[dir][j + 1]);
		std::list<std::pair<GPSPoint*, BusLineUnit*>*>* unit_list = busline_manager->line_index->GetSegment(segment, 1);

		for (auto iter = unit_list->begin(); iter != unit_list->end(); iter++) {
			GPSPoint* point = (*iter)->first;
			double to_start_distance = line->routes[dir][j].GPSDistance(*(point));
			if (to_start_distance < left_length)
				line_front_points.insert(*(point));
		}

		delete unit_list;
		if (now_front_total_length > LINE_FRONT_LENGTH) {
			break;
		}
	}

	return line_front_points;
}

int LineFrontSimilar::getIntersectionCount(set<GPSPoint, GPSPointCompare>& a, set<GPSPoint, GPSPointCompare>& b) {
	int count = 0;
	for (auto iter = a.begin(); iter != a.end(); iter++) {
		if (b.find(*iter) != b.end()) {
			count += 1;
		}
	}
	return count;
}

bool LineFrontSimilar::isStartNear(int dir_serial1, int dir_serial2, BusLineManager* busline_manager) {
	int dir1 = dir_serial1 % 2;
	int serial1 = dir_serial1 >> 1;
	BusLine* line1 = busline_manager->GetLine(serial1);

	int dir2 = dir_serial2 % 2;
	int serial2 = dir_serial2 >> 1;

	BusLine* line2 = busline_manager->GetLine(serial2);

	GPSPoint point1 = line1->stations[dir1][0].point;
	GPSPoint point2 = line2->stations[dir2][0].point;

	if (point1.GPSDistance(point2) < 200)
		return true;
	else
		return false;


}
