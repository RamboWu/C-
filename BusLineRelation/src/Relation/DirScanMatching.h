#ifndef DIR_SCAN_MATCHING_H
#define DIR_SCAN_MATCHING_H

#include <boost/filesystem.hpp>
#include "RoutesMatching.h"
#include "StationMatching.h"
#include "MatchingResult.h"
#include "../Bus/BusLineManager.h"

class DirScanMatching {

	RoutesMatching* routes_matching;
	StationMatching* station_matching;
	BusLineManager* line_manager;
	LineReport* line_report;

	FILE* matching_out;
	FILE* result_out;

	std::string GetLineSetString(std::set<int> &lineSet) {

		std::string str = "[";
		int counter = 0;
		for (auto iter = lineSet.begin(); iter != lineSet.end(); iter++) {
			if (counter > 0) str = str + "|";
			str += line_manager->GetLine(*iter)->line_id;
			counter++;
		}
		str += "]";
		return str;
	}

	void CalcBusMatching(std::vector<BusData*> & bus) {
		if (bus.size() == 0) return;
		std::string busId = bus[0]->bus_id;

		MatchingResult* match_result = new MatchingResult;
		routes_matching->GetMatchingRate(bus, match_result, 2);
		station_matching->GetMatchingRate(bus, match_result);

		int max_bus_to_line = -1;
		int max_line_to_bus = -1;
		int max_station_to_bus = -1;

		std::set<int> max_bus_to_line_total;
		std::set<int> max_line_to_bus_total;
		std::set<int> max_station_to_bus_total;

		for (auto iter = match_result->bus_to_line_num.begin(); iter != match_result->bus_to_line_num.end(); iter++) {
			if (max_bus_to_line == -1 || iter->second > match_result->bus_to_line_num[max_bus_to_line]) {
				max_bus_to_line = iter->first;
				max_bus_to_line_total.clear();
				max_bus_to_line_total.insert(iter->first / 2);
			}
			else if (max_bus_to_line != -1 && iter->second == match_result->bus_to_line_num[max_bus_to_line] && max_bus_to_line_total.find(iter->first / 2) == max_bus_to_line_total.end()) {
				max_bus_to_line_total.insert(iter->first / 2);
			}
		}

		for (auto iter = match_result->line_to_bus_num.begin(); iter != match_result->line_to_bus_num.end(); iter++) {
			if (max_line_to_bus == -1 || iter->second > match_result->line_to_bus_num[max_line_to_bus]) {
				max_line_to_bus = iter->first;
				max_line_to_bus_total.clear();
				max_line_to_bus_total.insert(iter->first / 2);
			}
			else if (max_line_to_bus != -1 && iter->second == match_result->line_to_bus_num[max_line_to_bus] && max_line_to_bus_total.find(iter->first / 2) == max_line_to_bus_total.end()) {
				max_line_to_bus_total.insert(iter->first / 2);
			}
		}

		for (auto iter = match_result->station_to_bus_num.begin(); iter != match_result->station_to_bus_num.end(); iter++) {
			if (max_station_to_bus == -1 || iter->second > match_result->station_to_bus_num[max_station_to_bus]) {
				max_station_to_bus = iter->first;
				max_station_to_bus_total.clear();
				max_station_to_bus_total.insert(iter->first / 2);
			}
			else if (max_station_to_bus != -1 && iter->second == match_result->station_to_bus_num[max_station_to_bus] && max_station_to_bus_total.find(iter->first / 2) == max_station_to_bus_total.end()) {
				max_station_to_bus_total.insert(iter->first / 2);
			}
		}

		std::string max_bus_to_line_str = GetLineSetString(max_bus_to_line_total);
		std::string max_line_to_bus_str = GetLineSetString(max_line_to_bus_total);
		std::string max_station_to_bus_str = GetLineSetString(max_station_to_bus_total);

		fprintf(matching_out, 
			"%s,%s,%.2lf,%d,%s,%.2lf,%d,%s,%.2lf,%d\n",
			busId.c_str(),
			max_bus_to_line_str.c_str(),
			match_result->bus_to_line[max_bus_to_line],
			max_bus_to_line_total.size(),
			max_line_to_bus_str.c_str(),
			match_result->line_to_bus[max_line_to_bus],
			max_line_to_bus_total.size(),
			max_station_to_bus_str.c_str(),
			match_result->station_to_bus[max_station_to_bus],
			max_station_to_bus_total.size()
			);

		fflush(matching_out);

		if (max_bus_to_line_total.size() == 1 && 
			max_line_to_bus_total.size() == 1 && 
			max_bus_to_line_str == max_line_to_bus_str && 
			match_result->bus_to_line[max_bus_to_line]>=0.9 && 
			match_result->line_to_bus[max_line_to_bus]>=0.9) {
			fprintf(result_out, "%s,%s\n",
				busId.c_str(),
				max_bus_to_line_str.c_str()
				);
			fflush(result_out);
		}

		

		delete match_result;
	}

public:

	DirScanMatching(const char* line_file,const char* matching_file,const char* result_file) {
		line_report = new LineReport;
		line_manager = new BusLineManager;
		line_manager->Load(line_file, line_report);
		routes_matching = new RoutesMatching(line_manager);
		station_matching = new StationMatching(line_manager);

		matching_out = fopen(matching_file, "w");
		result_out = fopen(result_file, "w");
	}

	~DirScanMatching() {
		delete line_report;
		delete line_manager;
		delete routes_matching;
		delete station_matching;
	}


	void DirScan(const char* dir) {
		boost::filesystem::path dir_path(dir);
		boost::filesystem::directory_iterator end_iter;

		for (auto iter = boost::filesystem::directory_iterator(dir_path); iter != end_iter; iter++) {

			for (auto iter1 = boost::filesystem::directory_iterator(iter->path()); iter1 != end_iter; iter1++) {
				std::cout << iter1->path().string() << std::endl;

				std::vector<BusData*> bus;
				std::ifstream in(iter1->path().string());
				std::string temp;
				while (getline(in, temp)) {
					BusData* bus_data = new BusData;
					bus_data->parseSimple(temp);
					bus.push_back(bus_data);
				}
				in.close();
				CalcBusMatching(bus);

				for (int i = 0; i < bus.size(); i++) delete bus[i];
				bus.clear();
			}
		}
	}

	void FileMatching(const char* file) {

		std::ifstream in(file);

		std::string pre_busId = "";
		std::vector<BusData*> bus;
		std::string temp;
		while (getline(in, temp)) {
			BusData* bus_data = new BusData;
			bus_data->parse(temp);
			if (bus_data->bus_id != pre_busId) {
				if (bus.size()>0) {
					CalcBusMatching(bus);
				}
				pre_busId = bus_data->bus_id;
				for (int i = 0; i < bus.size(); i++) delete bus[i];
				bus.clear();
			}
			bus.push_back(bus_data);
		}

		if (bus.size()>0) {
			CalcBusMatching(bus);
		}
		for (int i = 0; i < bus.size(); i++) delete bus[i];
		bus.clear();
		in.close();

	}

	void Close() {
		fclose(matching_out);
		fclose(result_out);
	}

};

#endif