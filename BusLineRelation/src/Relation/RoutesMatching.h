#ifndef ROUTES_MATCHING_H
#define ROUTES_MATCHING_H

#include "../Index/PlaneIndex.h"
#include "../Bus/BusLineIndexInfo.h"
#include "../Bus/Bus.h"
#include "../Bus/BusLineManager.h"
#include "MatchingResult.h"

class RoutesMatching {

	BusLineManager* busline_manager;

public:

	RoutesMatching(BusLineManager* busline_manager) {
		this->busline_manager = busline_manager;
	}


	void GetMatchingRate(std::vector<BusData*> &bus, MatchingResult* match_result, int vague) {
		map<int, set<GPSPoint, GPSPointCompare>*> bus_counts;
		map<int, set<GPSPoint, GPSPointCompare>*> line_counts;
		set<GPSPoint, GPSPointCompare> bus_point_set;

		for (int i = 0; i < bus.size(); i++) {
			
			GPSPoint point = GPSPoint(busline_manager->line_index->GetLonLeftBorder(bus[i]->point.lon), busline_manager->line_index->GetLatLeftBorder(bus[i]->point.lat));
			bus_point_set.insert(point);

			std::set<int> near_line;
			std::list<pair<GPSPoint*, BusLineUnit*>* > * unit_list = busline_manager->line_index->GetPoint(bus[i]->point, vague + 1);

			if (unit_list == NULL) {
				continue;
			}
			
			for (auto iter = unit_list->begin(); iter != unit_list->end(); iter++) {
				if (*iter == NULL) continue;
				for (auto iter1 = (*iter)->second->line_dir_sets.begin(); iter1 != (*iter)->second->line_dir_sets.end(); iter1++) {
					int line_index = iter1->first;
					near_line.insert(line_index);

					if (line_counts.find(line_index) == line_counts.end()) {
						set<GPSPoint, GPSPointCompare>* point_set = new set<GPSPoint, GPSPointCompare>();
						line_counts[line_index] = point_set;
					}
					line_counts[line_index]->insert(*((*iter)->first));
				}
			}

			for (auto iter = near_line.begin(); iter != near_line.end(); iter++) {
				if (bus_counts.find(*iter) == bus_counts.end()) {
					set<GPSPoint, GPSPointCompare>* point_set = new set<GPSPoint, GPSPointCompare>();
					bus_counts[*iter] = point_set;
				}
				bus_counts[*iter]->insert(point);
			}

			delete unit_list;
		}

		for (auto iter = line_counts.begin(); iter != line_counts.end(); iter++) {
			match_result->bus_to_line[iter->first] = (double)iter->second->size() / busline_manager->line_point_in_index[iter->first]->size();
			match_result->bus_to_line_num[iter->first] = iter->second->size();
			if (bus_counts.find(iter->first) != bus_counts.end()) {
				match_result->line_to_bus[iter->first] = (double)bus_counts[iter->first]->size() / bus_point_set.size();
				match_result->line_to_bus_num[iter->first] = bus_counts[iter->first]->size();
			}
			else {
				match_result->line_to_bus[iter->first] = 0;
				match_result->line_to_bus_num[iter->first] = 0;
			}
		}

		for (auto iter = line_counts.begin(); iter != line_counts.end(); iter++) {
			delete iter->second;
		}

		for (auto iter = bus_counts.begin(); iter != bus_counts.end(); iter++) {
			delete iter->second;
		}
	}

};

#endif