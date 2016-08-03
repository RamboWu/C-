#include "LineFrontSimilar.h"

void LineFrontSimilar::CalcSimilar(BusLineManager* busline_manager) {

	Calc(busline_manager);
	Report();

}

void LineFrontSimilar::Calc(BusLineManager* busline_manager) {
	std::map<int, std::set<int>> similar;

	//求出similar数组,与线first相似的有set个
	for (int i = 0; i < busline_manager->line_set.size(); i++) {
		BusLine* line = busline_manager->line_set[i];

		std::map<int, int> temp_similar[2];

		for (int k = 0; k < line->dir_num; k++) {

			std::map<int, set<GPSPoint, GPSPointCompare>> line_similar;

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

				SimilarMeta similar_meta;
				similar_meta.first = line->serial * 2 + k;
				similar_meta.second = iter->first;

				similar_rate[similar_meta] = (double)size / busline_manager->line_point_in_index[iter->first]->size();

				if (size >= busline_manager->line_point_in_index[iter->first]->size() * 0.98) {
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

	std::set<std::string> contain;

	for (auto iter = similar.begin(); iter != similar.end(); iter++) {

		int line_id = iter->first;
		BusLine* line = busline_manager->GetLine(line_id);
		if (contain.find(line->line_id) != contain.end()) continue;
		std::vector<std::pair<std::string, std::string>> result;
		result.push_back(std::pair<std::string, std::string>(line->line_id, line->line_name));

		for (auto iter1 = iter->second.begin(); iter1 != iter->second.end(); iter1++) {

			BusLine* line1 = busline_manager->GetLine(*iter1);

			if (similar.find(*iter1) != similar.end() && similar[*iter1].find(line_id) != similar[*iter1].end()) {
				result.push_back(std::pair<std::string, std::string>(line1->line_id, line1->line_name));
			}
		}

		if (result.size() > 1) {
			line_report->AddSimilarLine(result);
			for (int i = 0; i < result.size(); i++)
				contain.insert(result[i].first);
		}

	}
}
