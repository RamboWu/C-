#pragma once

#ifndef _BUS_LINE_INDEX_INFO_H_
#define _BUS_LINE_INDEX_INFO_H_

#include <stdint.h>
#include <vector>
#include <map>
#include "../Index/PlaneIndex.h"

class BusLineInPlaneUnit {
public:
	GPSPoint start, end;
	int dir;
	int route_serial;
	int line_serial;
	std::string line_id;
	std::string line_name;
};

class BusLineMetadata {
public:
	int line_serial;
	int dir;
	int index;

	BusLineMetadata(int line_serial, int dir, int index) {
		this->line_serial = line_serial;
		this->dir = dir;
		this->index = index;
	}

	~BusLineMetadata() {
	}
};


class BusLineMetadataCMP {
public:
	bool operator() (const BusLineMetadata &a, const BusLineMetadata &b) {
		if (a.line_serial < b.line_serial) return true;
		if (a.line_serial > b.line_serial) return false;
		if (a.dir < b.dir) return true;
		if (a.dir > b.dir) return false;
		if (a.index < b.index) return true;
		return false;
	}
};

class BusLineUnit {
public:
	double min_lat, min_lon, max_lat, max_lon;
	std::map<int, std::vector<BusLineInPlaneUnit*>> line_dir_sets;
	std::map<int, bool> line_map;

	~BusLineUnit() {
		for (auto iter = line_dir_sets.begin(); iter != line_dir_sets.end(); iter++) {
			iter->second.clear();
		}
		line_dir_sets.clear();
	}
};


#endif

