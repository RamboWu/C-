#include "Bus.h"
#include "../Util/StringUtils.h"

int BusData::counter = 0;

bool BusData::parseSimple(std::string& str) {
	std::vector<std::string> tags;
	StringUtils::split(tags, str, ",");

	bus_id = tags[0];
	point.lon = atof(tags[1].c_str());
	point.lat = atof(tags[2].c_str());
	return true;

}

void BusData::parse(std::string& str) {
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

void BusData::parseWithIndex(std::string& str) {

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

void BusData::ToOutput(FILE * fout) {
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

void BusData::ToOutputWithIndex(FILE * fout) {
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