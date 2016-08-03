#ifndef BUS_SIMILAR_H
#define BUS_SIMILAR_H

#include <set>
#include "../Bus/Bus.h"
#include "../Report/BusReport.h"
#include "../Index/PlaneIndex.h"
#include "../Bus/BusLineManager.h"
#include "../Util/Configuration.h"

class BusIndexInfo {
public:
	std::set<int> busSets;
};

class BusSimilar {

	int bus_total=0;

	std::map<int, std::set<int>> similar;

	std::map<std::string, int> bus_map1;
	std::map<int, std::string> bus_map2;
	std::map<int, std::set<GPSPoint,GPSPointCompare>> bus_point_total;

	BusReport* bus_report;
	BusLineManager* busline_manager;

	PlaneIndex<BusIndexInfo> *bus_index;

public:

	BusSimilar(BusReport* bus_report,BusLineManager* busline_manager) {
		this->bus_report = bus_report;
		this->busline_manager = busline_manager;

		bus_index = new PlaneIndex<BusIndexInfo>(
			busline_manager->min_lon,
			busline_manager->min_lat,
			busline_manager->max_lon,
			busline_manager->max_lat,
			1500
		);
	}

	~BusSimilar() {
		delete bus_index;
	}

	void InsertBus(std::vector<BusData*> & bus) {

		if (Configuration::line_mind == 0) return;

		std::string busId = bus[0]->bus_id;
		if (bus_map1.find(busId) == bus_map1.end()) {
			++bus_total;
			bus_map1[busId] = bus_total;
			bus_map2[bus_total] = busId;
			std::set<GPSPoint, GPSPointCompare> temp;
			bus_point_total[bus_total] = temp;
		}

		int bus_serial = bus_map1[busId];

		for (int i = 0; i < bus.size(); i++) {
			
			std::list<std::pair<GPSPoint*, BusIndexInfo*>*>* unit = bus_index->GetPoint(bus[i]->point, 1);

			if (unit != NULL) {
				for (auto iter = unit->begin(); iter != unit->end(); iter++) {
					(*iter)->second->busSets.insert(bus_serial);
					bus_point_total[bus_serial].insert(*(*iter)->first);
				}
			}
		}
	}

	void CalcSimilar(std::vector<BusData*> & bus) {

		if (Configuration::line_mind == 0) return;

		std::string busId = bus[0]->bus_id;
		if (bus_map1.find(busId) == bus_map1.end()) {
			return;
		}

		int bus_serial = bus_map1[busId];

		std::map<int, std::set<GPSPoint, GPSPointCompare>> similar_point;

		GPSPoint left, right;
		left.lat = 200;
		left.lon = 200;
		right.lat = 0;
		right.lon = 0;

		for (int i = 0; i < bus.size(); i++) {

			std::list<std::pair<GPSPoint*, BusIndexInfo*>*>* unit = bus_index->GetPoint(bus[i]->point,2);

			if (bus[i]->point.lat > right.lat && bus[i]->point.lon > right.lon) right = bus[i]->point;
			if (bus[i]->point.lat < left.lat && bus[i]->point.lon < left.lon) left = bus[i]->point;

			if (unit != NULL) {
				for (auto iter = unit->begin(); iter != unit->end(); iter++) {

					for (auto iter1 = (*iter)->second->busSets.begin(); iter1 != (*iter)->second->busSets.end(); iter1++) {
						similar_point[*iter1].insert(*(*iter)->first);
					}

				}
			}
			
			delete unit;
		}

		long dis = left.GPSDistance(right);
		if (dis <= 1000) return;

		for (auto iter = similar_point.begin(); iter != similar_point.end(); iter++) {

			int size = iter->second.size();

			if (size >= bus_point_total[iter->first].size() * 0.90 && iter->first!=bus_serial) {
				if (similar.find(bus_serial) == similar.end()) {
					std::set<int> temp;
					similar[bus_serial] = temp;
				}
				similar[bus_serial].insert(iter->first);
			}
		}
	}

	void Summary() {

		if (Configuration::line_mind == 0) return;

		std::set<std::string> contain;

		for (auto iter = similar.begin(); iter != similar.end(); iter++) {

			std::vector<std::string> result;
			result.push_back(bus_map2[iter->first]);

			if (contain.find(bus_map2[iter->first])!=contain.end()) continue;

			for (auto iter1 = iter->second.begin(); iter1 != iter->second.end(); iter1++) {

				if (similar.find(*iter1) != similar.end() && similar[*iter1].find(iter->first) != similar[*iter1].end() && contain.find(bus_map2[*iter1]) == contain.end()) {
					result.push_back(bus_map2[*iter1]);
				}
			}

			if (result.size() > 1) {
				bus_report->AddSimilarBus(result);

				for (int i = 0; i < result.size(); i++)
					contain.insert(result[i]);
			}

		}

	}
};

#endif