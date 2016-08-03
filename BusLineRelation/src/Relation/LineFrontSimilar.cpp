#include "LineFrontSimilar.h"
#include "../Bus/BusLineManager.h"

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
			set<GPSPoint, GPSPointCompare> line_front_points;
			double now_front_total_length = 0;
			//求得与当前dir_serial有相交的所有line
			for (int j = 0; j < (int)line->routes[k].size() - 1; j++) {

				double length = line->routes[k][j].GPSDistance(line->routes[k][j + 1]);
				now_front_total_length += length;
				if (now_front_total_length > 500)
					break;

				Segment segment = Segment(line->routes[k][j], line->routes[k][j + 1]);
				std::list<std::pair<GPSPoint*, BusLineUnit*>*>* unit_list = busline_manager->line_index->GetSegment(segment, 1);

				for (auto iter = unit_list->begin(); iter != unit_list->end(); iter++) {
					GPSPoint* point = (*iter)->first;
					line_front_points.insert(*(point));

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

				SimilarMeta similar_meta;
				similar_meta.first = line->serial * 2 + k;
				similar_meta.second = iter->first;

				similar_rate[similar_meta] = (double)size / line_front_points.size();

				if (size >= line_front_points.size() * 0.98) {
					if (temp_similar[k].find(line_serial) == temp_similar[k].end()) {
						temp_similar[k][line_serial] = 0;
					}
					temp_similar[k][line_serial] += 1 << dir;
				}
			}
		}
		
		if (line->dir_num == 1) {
			std::vector<int> temp;
			for (auto iter = temp_similar[0].begin(); iter != temp_similar[0].end(); iter++) {

				BusLine* temp_line = busline_manager->GetLine(iter->first);
				if (iter->second == 1) {
					temp.push_back(temp_line->serial);
				}

			}
			if (temp.size() > 0) {
				if (similar.find(line->serial) == similar.end()) {
					std::set<int> t;
					similar[line->serial] = t;
				}
				for (int t = 0; t < temp.size(); t++) {
					similar[line->serial].insert(temp[t]);
				}
			}

		}
		else {

			std::vector<int> temp;
			for (auto iter = temp_similar[0].begin(); iter != temp_similar[0].end(); iter++) {

				BusLine* temp_line = busline_manager->GetLine(iter->first);
				if (temp_similar[1].find(iter->first) != temp_similar[1].end()) {
					int value = temp_similar[1][iter->first];
					if ((iter->second | value == 3) && (iter->second ^ value == 3) && iter->second>0 && value > 0) {
						temp.push_back(temp_line->serial);
					}
				}
			}
			if (temp.size() > 0) {
				if (similar.find(line->serial) == similar.end()) {
					std::set<int> t;
					similar[line->serial] = t;
				}
				for (int t = 0; t < temp.size(); t++) {
					similar[line->serial].insert(temp[t]);
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
		fprintf(temp_out, "%s: ", line->line_id.c_str());
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
