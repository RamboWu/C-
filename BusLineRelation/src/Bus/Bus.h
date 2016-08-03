#pragma once

#ifndef _BUS_BUS_H_
#define _BUS_BUS_H_

#include <string>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <json/json.h>
#include "../Util/DateTime.h"
#include "../Index/PlaneIndex.h"


class BusData {
public:

	int status, bus_status, is_error;
	std::string bus_id;
	std::string gps_time, recv_time;
	GPSPoint point;
	std::string oline_id, line_id, line_name;
	int dir, line_status;
	double last_dis;
	int next_station;
	double next_station_dis, prev_station_dis;

	int index;

	int serial;

	static int counter;

	std::string data;

	bool parseSimple(std::string& str);

	void parse(std::string& str);

	void parseWithIndex(std::string& str);

	void ToOutput(FILE * fout);

	void ToOutputWithIndex(FILE * fout);

};

class BusDataCompareOpt {
public:
	bool operator() (const BusData* a, const BusData* b) {
		if (a->bus_id < b->bus_id) return true;
		if (a->bus_id > b->bus_id) return false;
		if (a->index < b->index) return true;
		return false;
	}
};


class BusStation {
public:
	int index;
	std::string name;
	GPSPoint point;
};

class BusLine {
public:
	int serial;
	std::string line_id;
	std::string line_name;
	std::string start_time;
	std::string end_time;
	int dir_num;
	int station_num[2];
	double routes_length[2];
	std::vector<GPSPoint> routes[2];
	std::vector<double> sum_routes[2];
	std::vector<double> station_dis[2];
	std::vector<double> station_dis_u[2];
	std::vector<BusStation> stations[2];

	bool Parse(int serial, const std::string& str) {
		Json::Reader reader;
		Json::Value value;

		this->serial = serial;

		memset(station_num, 0, 2*sizeof(int));
		if (reader.parse(str, value)) {
			if (!value["number"].isNull()) {
				this->line_id = value["number"].asString();
			}
			else {
				return false;
			}

			if (!value["name"].isNull()) {
				this->line_name = value["name"].asString();
			}
			else {
				return false;
			}

			if (!value["start"].isNull()) {
				this->start_time = value["start"].asString();
			}

			if (!value["end"].isNull()) {
				this->end_time = value["end"].asString();
			}

			this->dir_num = 0;

			int station_flag = 0;
			for (int k = 0; k < 2; k++) {
				std::string tmp_str = "stations";
				tmp_str += (char)(k + 49);
				if (!value[tmp_str].isNull()) {
					int routes_size = value[tmp_str].size();
					for (int i = 0; i < routes_size; i++) {
						BusStation bus_station;
						bus_station.index = value[tmp_str][i]["index"].asInt();
						bus_station.name = value[tmp_str][i]["name"].asString();
						bus_station.point.lon = value[tmp_str][i]["lon"].asDouble();
						bus_station.point.lat = value[tmp_str][i]["lat"].asDouble();
						GPSPoint boder_lng = GPSPoint(0, bus_station.point.lon);
						GPSPoint boder_lat = GPSPoint(0, bus_station.point.lat);
						this->stations[this->dir_num].push_back(bus_station);
					}
					if (routes_size > 0) {
						this->dir_num++;
						station_flag += 1 << k;
					}
				}
			}

			if (this->dir_num == 0) return false;

			int temp_dir = 0;
			int routes_flag = 0;
			for (int k = 0; k < 2; k++) {
				this->routes[temp_dir].clear();
				std::string tmp_str = "routes";
				tmp_str += (char)(k + 49);
				if (!value[tmp_str].isNull()) {
					int routes_size = value[tmp_str].size();
					for (int i = 0; i < routes_size; i++) {
						GPSPoint gps_point;
						gps_point.lon = value[tmp_str][i]["lon"].asDouble();
						gps_point.lat = value[tmp_str][i]["lat"].asDouble();
						if (i == 0) {
							this->routes[temp_dir].push_back(gps_point);
						}
						else if (gps_point.lon != this->routes[temp_dir][this->routes[temp_dir].size() - 1].lon || gps_point.lat != this->routes[temp_dir][this->routes[temp_dir].size() - 1].lat) {
							this->routes[temp_dir].push_back(gps_point);
						}
					}
					if (routes_size > 0) {
						temp_dir++;
						routes_flag += 1 << k;
					}
				}
			}

			if (temp_dir != this->dir_num || station_flag != routes_flag || station_flag == 2 || routes_flag == 2) return false;
			return true;
		}
		return false;
	}
};

#endif
