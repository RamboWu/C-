#ifndef STATION_MATCHING_H
#define STATION_MATCHING_H

#include "../Index/PlaneIndex.h"
#include "../Bus/BusLineIndexInfo.h"
#include "../Bus/Bus.h"
#include "../Bus/BusLineManager.h"
#include "MatchingResult.h"
#include "../Util/GPSTools.h"


class StationMatching{
	BusLineManager* busline_manager;

	int** match_buffer;
	int max_station = 0;
	int max_bus_size = 0;

	void Matching(BusLine* line, int dir, std::vector<BusData*> &bus, MatchingResult* match_result) {
		if (line->station_num[dir] == 0) return;

		int k = 0;
		for (int i = 0; i < bus.size(); i++) {
			int e = (i == 0) ? line->station_num[dir] : match_buffer[k][i - 1];
			match_buffer[k][i] = e;
			for (int j = 0; j < e; j++)
				if (GPSTools::GPSEqual(&line->stations[dir][j].point, &bus[i]->point)) {
					match_buffer[k][i] = j;
					break;
				}
		}

		while (k < line->station_num[dir] && match_buffer[k][bus.size() - 1] < line->station_num[dir]) {
			k++;

			for (int i = 0; i < k; i++) match_buffer[k][i] = line->station_num[dir];

			for (int i = k; i < bus.size(); i++) {
				int e = (i == k) ? line->station_num[dir] : match_buffer[k][i - 1];
				match_buffer[k][i] = e;
				for (int j = match_buffer[k - 1][i - 1] + 1; j < e; j++)
					if (GPSTools::GPSEqual(&line->stations[dir][j].point, &bus[i]->point)) {
						match_buffer[k][i] = j;
						break;
					}
			}
		}

		match_result->station_to_bus[line->serial * 2 + dir] = (double)k / line->station_num[dir];
		match_result->station_to_bus_num[line->serial * 2 + dir] = k;
	}

public:
	
	StationMatching(BusLineManager* busline_manager) {
		this->busline_manager = busline_manager;
		max_station = busline_manager->max_station + 1;
		match_buffer = new int*[max_station];
	}

	~StationMatching() {
		if (max_bus_size > 0) {
			for (int i = 0; i < max_station; i++) {
				delete[] match_buffer[i];
			}
		}
		delete[] match_buffer;
	}

	void GetMatchingRate(std::vector<BusData*> &bus, MatchingResult* match_result) {

		if (bus.size() > max_bus_size) {
			if (max_bus_size > 0) {
				for (int i = 0; i < max_station; i++) {
					delete[] match_buffer[i];
				}
			}
			for (int i = 0; i < max_station; i++)
				match_buffer[i] = new int[bus.size()+1];
			max_bus_size = bus.size();
		}


		for (int i = 0; i < busline_manager->line_total;i++) {
			BusLine* line = busline_manager->GetLineByNum(i);
			for (int k = 0; k < line->dir_num; k++) {
				int count = 0;

				if (line->station_num[k]>0 && (match_result->line_to_bus[line->serial * 2 + k] >= 0.8 || match_result->bus_to_line[line->serial * 2 + k] >= 0.8)) {
					Matching(line, k,bus,match_result);
				}
			}
		}
	}
};

#endif