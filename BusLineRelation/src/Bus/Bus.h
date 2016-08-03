#pragma once

#ifndef _BUS_BUS_H_
#define _BUS_BUS_H_

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <json/json.h>
#include "../Util/DateTime.h"
#include "../Index/PlaneIndex.h"

#ifndef _ANALYSE_BUS_H_
#define _ANALYSE_BUS_H_

#include <string>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "../Util/StringUtils.h"
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

	bool parseSimple(std::string& str) {
		std::vector<std::string> tags;
		StringUtils::split(tags, str, ",");

		bus_id = tags[0];
		point.lon = atof(tags[1].c_str());
		point.lat = atof(tags[2].c_str());
		return true;

	}

	void parse(std::string& str) {
		std::vector<std::string> tags;
		StringUtils::split(tags, str, ",");

		status = atoi(tags[0].c_str());
		bus_status = atoi(tags[1].c_str());
		is_error = atoi(tags[2].c_str());
		bus_id = tags[3];
		line_id = tags[4];
		line_name = tags[5];
		dir = atoi(tags[6].c_str());
		next_station = atoi(tags[7].c_str());
		line_status = atoi(tags[8].c_str());
		last_dis = atof(tags[9].c_str());
		point.lon = atof(tags[10].c_str());
		point.lat = atof(tags[11].c_str());
		oline_id = tags[12];
		gps_time = tags[13];
		recv_time = tags[14];
		prev_station_dis = atof(tags[15].c_str());
		next_station_dis = atof(tags[16].c_str());

		BusData::counter++;
		index = counter;
	}

	void parseWithIndex(std::string& str) {

		std::vector<std::string> tags;
		StringUtils::split(tags, str, ",");

		status = atoi(tags[0].c_str());
		bus_status = atoi(tags[1].c_str());
		is_error = atoi(tags[2].c_str());
		bus_id = tags[3];
		line_id = tags[4];
		line_name = tags[5];
		dir = atoi(tags[6].c_str());
		next_station = atoi(tags[7].c_str());
		line_status = atoi(tags[8].c_str());
		last_dis = atof(tags[9].c_str());
		point.lon = atof(tags[10].c_str());
		point.lat = atof(tags[11].c_str());
		oline_id = tags[12];
		gps_time = tags[13];
		recv_time = tags[14];
		prev_station_dis = atof(tags[15].c_str());
		next_station_dis = atof(tags[16].c_str());

		index = atoi(tags[18].c_str());
	}

	void ToOutput(FILE * fout) {
		fprintf(fout,
			"%d,%d,%d,%s,%s,%s,%d,%d,%d,%.2lf,%.6lf,%.6lf,%s,%s,%s,%.2lf,%.2lf\n",
			status,
			bus_status,
			is_error,
			bus_id.c_str(),
			line_id.c_str(),
			line_name.c_str(),
			dir,
			next_station,
			line_status,
			last_dis,
			point.lon,
			point.lat,
			oline_id.c_str(),
			gps_time.c_str(),
			recv_time.c_str(),
			prev_station_dis,
			next_station_dis
		);
	}

	void ToOutputWithIndex(FILE * fout) {
		fprintf_s(fout,
			"%d,%d,%d,%s,%s,%s,%d,%d,%d,%.2lf,%.6lf,%.6lf,%s,%s,%s,%.2lf,%.2lf,%d\n",
			status,
			bus_status,
			is_error,
			bus_id.c_str(),
			line_id.c_str(),
			line_name.c_str(),
			dir,
			next_station,
			line_status,
			last_dis,
			point.lon,
			point.lat,
			oline_id.c_str(),
			gps_time.c_str(),
			recv_time.c_str(),
			prev_station_dis,
			next_station_dis,
			index
		);
	}

};

int BusData::counter = 0;

class BusDataCompareOpt {
public:
	bool operator() (const BusData* a, const BusData* b) {
		if (a->bus_id < b->bus_id) return true;
		if (a->bus_id > b->bus_id) return false;
		if (a->index < b->index) return true;
		return false;
	}
};

#endif

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
