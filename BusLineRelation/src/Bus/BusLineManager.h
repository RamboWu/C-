#pragma once

#ifndef _BUS_LINE_MANAGER_H_
#define _BUS_LINE_MANAGER_H_

#include <map>
#include <fstream>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h> 
#include <log4cplus/helpers/stringhelper.h>  
#include <log4cplus/loggingmacros.h>
#include "../Bus/Bus.h"
#include "../Util/Configuration.h"
#include "../Util/MileagesTools.h"
#include "../Util/StringUtils.h"
#include "../Report/LineReport.h"

class BusLineManager {
	std::map<int, BusLine*> line_map;
	std::map<std::string, int> line_name_to_serial;
	std::map<std::string, int> line_id_to_serial;

	void BuildIndex(LineReport* line_report) {

		line_index = new PlaneIndex<BusLineUnit>(
			min_lon,
			min_lat,
			max_lon,
			max_lat,
			Configuration::index_error
			);

		for (int i = 0; i < line_set.size(); i++) {
			BusLine * line = line_set[i];
			for (int k = 0; k < line_set[i]->dir_num; k++) {
				for (int j = 0; j < (int)line_set[i]->routes[k].size() - 1; j++) {
					BusLineInPlaneUnit* plane_unit = new BusLineInPlaneUnit;
					plane_unit->line_id = line->line_id;
					plane_unit->line_serial = line->serial;
					plane_unit->line_name = line->line_name;
					plane_unit->dir = k;
					plane_unit->route_serial = j + 1;
					plane_unit->start.lon = line->routes[k][j].lon;
					plane_unit->start.lat = line->routes[k][j].lat;
					plane_unit->end.lon = line->routes[k][j + 1].lon;
					plane_unit->end.lat = line->routes[k][j + 1].lat;

					Segment segment = Segment(line->routes[k][j], line->routes[k][j + 1]);
					std::list<std::pair<GPSPoint*, BusLineUnit*>*>* unit_list = line_index->GetSegment(segment,1);

					for (auto iter = unit_list->begin(); iter != unit_list->end(); iter++) {
						(*iter)->second->line_dir_sets[plane_unit->line_serial * 2 + plane_unit->dir].push_back(plane_unit);
						(*iter)->second->line_map[plane_unit->line_serial] = true;

						int line_id = plane_unit->line_serial * 2 + plane_unit->dir;
						if (line_point_in_index.find(line_id) == line_point_in_index.end()) {
							line_point_in_index[line_id] = new set<GPSPoint, GPSPointCompare>();
						}
						line_point_in_index[line_id]->insert(*(*iter)->first);
					}
					unit_list->clear();
					delete unit_list;

				}

				line->sum_routes[k].push_back(0);
				for (int j = 1; j < line->routes[k].size(); j++) {
					line->sum_routes[k].push_back(line->sum_routes[k][j - 1] + line->routes[k][j].GPSDistance(line->routes[k][j - 1]));
				}

				if (line->routes[k].size()>0)
					line->routes_length[k] = line->sum_routes[k][line->sum_routes[k].size() - 1];
				else
					line->routes_length[k] = 0;

			}
		}

		line_index_judge = new PlaneIndex<BusLineUnit>(
			min_lon,
			min_lat,
			max_lon,
			max_lat,
			Configuration::index_error
			);

		for (int i = 0; i < line_set.size(); i++) {
			BusLine * line = line_set[i];
			for (int k = 0; k < line_set[i]->dir_num; k++) {
				for (int j = 0; j < (int)line_set[i]->routes[k].size() - 1; j++) {
					BusLineInPlaneUnit* plane_unit = new BusLineInPlaneUnit;
					plane_unit->line_id = line->line_id;
					plane_unit->line_serial = line->serial;
					plane_unit->line_name = line->line_name;
					plane_unit->dir = k;
					plane_unit->route_serial = j + 1;
					plane_unit->start.lon = line->routes[k][j].lon;
					plane_unit->start.lat = line->routes[k][j].lat;
					plane_unit->end.lon = line->routes[k][j + 1].lon;
					plane_unit->end.lat = line->routes[k][j + 1].lat;

					Segment segment = Segment(line->routes[k][j], line->routes[k][j + 1]);
					std::list<std::pair<GPSPoint*, BusLineUnit*>*>* unit_list = line_index_judge->GetSegment(segment, 2);

					for (auto iter = unit_list->begin(); iter != unit_list->end(); iter++) {
						(*iter)->second->line_dir_sets[plane_unit->line_serial * 2 + plane_unit->dir].push_back(plane_unit);
						(*iter)->second->line_map[plane_unit->line_serial] = true;
					}
					unit_list->clear();
					delete unit_list;

				}

			}
		}

		for (int i = 0; i < line_set.size(); i++) {
			for (int k = 0; k < line_set[i]->dir_num; k++) {
				if (line_set[i]->routes[k].size() > 0 && line_set[i]->stations[k].size() > 0) {

					double pre_dis = 0;
					for (int j = 0; j < line_set[i]->stations[k].size(); j++) {

						double now_dis = MileagesTools::GetPointMilesOnRoutes(line_set[i],
							k,
							line_set[i]->stations[k][j].point,
							pre_dis,
							10
						);

						double mis_dis = MileagesTools::GetPointToRoutesMinDistance(line_set[i],
							k,
							line_set[i]->stations[k][j].point,
							pre_dis
						);

						if (mis_dis > 10) {
							std::string station_index,dir; 
							StringUtils::ToString(k, dir);
							StringUtils::ToString(j + 1,station_index);
							line_report->AddFarAwayStation(line_set[i]->line_id, line_set[i]->line_name,dir+","+station_index);
							//cout << line_set[i]->line_id << " " << k+1 << " " << j << " "<< mis_dis << endl;
						}

						line_set[i]->station_dis[k].push_back(now_dis);
						line_set[i]->station_dis_u[k].push_back(now_dis);
						pre_dis = line_set[i]->station_dis[k][j];
					}

					vector<int> f, pre;
					f.push_back(1);
					pre.push_back(-1);

					for (int j = 1; j < line_set[i]->stations[k].size(); j++) {
						f.push_back(1);
						pre.push_back(-1);
						for (int l = 0; l <= j - 1; l++) {
							if (line_set[i]->station_dis_u[k][j] > line_set[i]->station_dis_u[k][l] && f[j] < f[l] + 1) {
								f[j] = f[l] + 1;
								pre[j] = l;
							}
						}
					}

					int j = line_set[i]->stations[k].size() - 1;
					while (j != -1) {
						if (j > pre[j] + 1) {
							for (int l = pre[j] + 1; l < j; l++)
								line_set[i]->station_dis_u[k][l] = pre[j] != -1 ? line_set[i]->station_dis_u[k][pre[j]] : 0;
						}
						j = pre[j];
					}
				}
			}
		}

		for (int i = 0; i < line_set.size(); i++) {
			bool flag = false;
			for (int k = 0; k < line_set[i]->dir_num; k++) {
				for (int j = 1; j < line_set[i]->stations[k].size(); j++) {
					if (line_set[i]->station_dis[k][j - 1] > line_set[i]->station_dis[k][j]) {
						flag = true;
						line_report->AddInvertingStation(line_set[i]->line_id,line_set[i]->line_name);
						break;
					}
				}
				if (flag) break;
			}
		}
	}

public:
	std::map<int, set<GPSPoint, GPSPointCompare>*> line_point_in_index;

	PlaneIndex<BusLineUnit>* line_index;

	PlaneIndex<BusLineUnit>* line_index_judge;


	std::map<int, std::string> line_serial_to_name;
	std::map<int, std::string> line_serial_to_id;
	std::vector<BusLine*> line_set;
	double min_lon = 1000, max_lon = -1000, min_lat = 1000, max_lat = -1000;
	int max_station = 0;
	int line_total = 0;

	BusLineManager() {
	}

	~BusLineManager() {
		for (int i = 0; i < line_set.size(); i++) {
			delete line_set[i];
		}

		delete line_index;
		delete line_index_judge;

		for (auto iter = line_point_in_index.begin(); iter != line_point_in_index.end(); iter++) {
			delete iter->second;
		}
	}

	void init() {
		line_map.clear();
		line_serial_to_name.clear();
		line_name_to_serial.clear();
		for (int i = 0; i < line_set.size(); i++) {
			delete line_set[i];
		}
		line_set.clear();
		min_lon = 1000, max_lon = -1000, min_lat = 1000, max_lat = -1000;
	}

	void Load(const char* line_input_file, LineReport* line_report) {
		init();

		std::string temp;
		std::ifstream bus_line_in(line_input_file);

		int line_serial = 0;
		while (getline(bus_line_in, temp)) {
			line_serial++;
			BusLine* bus_line = new BusLine;
			bool is_parse = bus_line->Parse(line_serial, temp);
			if (is_parse) {
				line_total++;
				line_set.push_back(bus_line);
				line_serial_to_id[line_serial] = bus_line->line_id;
				line_serial_to_name[line_serial] = bus_line->line_name;
				line_map[line_serial] = bus_line;
				line_name_to_serial[bus_line->line_name] = line_serial;
				line_id_to_serial[bus_line->line_id] = line_serial;

				for (int k = 0; k < bus_line->dir_num; k++) {
					bus_line->station_num[k] = bus_line->stations[k].size();
					for (int i = 0; i < bus_line->routes[k].size(); i++) {
						GPSPoint point = bus_line->routes[k][i];
						if (point.lon < min_lon) min_lon = point.lon;
						if (point.lat < min_lat) min_lat = point.lat;
						if (point.lon > max_lon) max_lon = point.lon;
						if (point.lat > max_lat) max_lat = point.lat;
					}
				}

				if (bus_line->station_num[0] > max_station) {
					max_station = bus_line->station_num[0];
				}
				if (bus_line->station_num[1] > max_station) {
					max_station = bus_line->station_num[1];
				}
			}
			else {
				line_report->AddFormatError(bus_line->line_id,bus_line->line_name);
				delete bus_line;
			}

		}
		bus_line_in.close();

		BuildIndex(line_report);
	}

	BusLine* GetLineByNum(int index) {
		if (line_set.size()>index && index>=0) {
			return line_set[index];
		}
		else {
			return NULL;
		}
	}

	BusLine* GetLine(int serial) {
		if (line_map.find(serial) != line_map.end()) {
			return line_map[serial];
		}
		else {
			return NULL;
		}
	}

	BusLine* GetLine(const std::string &line_id) {
		if (line_id_to_serial.find(line_id) != line_id_to_serial.end()) {
			int serial = line_id_to_serial[line_id];
			return line_map[serial];
		}
		else {
			return NULL;
		}
	}

	int GetSerialByLineId(const std::string &line_id) {
		if (line_id_to_serial.find(line_id) != line_id_to_serial.end()) {
			return line_id_to_serial[line_id];
		}
		else {
			return -1;
		}
	}

	double NextStationDis(int line_Id, int dir, double now_dis, int next_station) {
		next_station = next_station - 1;
		if (next_station >= 1 && line_map[line_Id]->station_dis_u[dir].size() > next_station) {
			return line_map[line_Id]->station_dis_u[dir][next_station] - now_dis;
		}
		else
			return 0;
	}

	double PrevStationDis(int line_Id, int dir, double now_dis, int prev_station) {
		prev_station = prev_station - 1;
		if (prev_station >= 0 && line_map[line_Id]->station_dis_u[dir].size() > prev_station) {
			return now_dis - line_map[line_Id]->station_dis_u[dir][prev_station];
		}
		else {
			return 0;
		}
	}

	int GetNextStation(int line_Id, int dir, double now_dis) {
		if (line_map[line_Id]->station_dis_u[dir].size() == 0) {
			cout << "Station Size is zero " << line_map[line_Id]->line_name<<endl;
		}
		int l = 0;
		int r = line_map[line_Id]->station_dis_u[dir].size() - 1;

		while (l < r - 1) {
			int mid = (l + r) / 2;
			if (line_map[line_Id]->station_dis_u[dir][mid] < now_dis) {
				l = mid;
			}
			else {
				r = mid;
			}
		}

		if (line_map[line_Id]->station_dis_u[dir][l] >= now_dis) return l + 1;
		if (line_map[line_Id]->station_dis_u[dir][r] >= now_dis) return r + 1;
		return -1;
	}

	std::string GetStationName(int line_Id, int dir, int station_index) {

		if (station_index>0 && station_index <= line_map[line_Id]->stations[dir].size()) {
			return line_map[line_Id]->stations[dir][station_index - 1].name;
		}
		else {
			return "-";
		}

	}
};

#endif