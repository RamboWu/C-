#ifndef _ANALYSE_STATION_JUDGE_H_
#define _ANALYSE_STATION_JUDGE_H_

#include "../Bus/Bus.h"
#include "../Util/MileagesTools.h"
#include "../Bus/Bus.h"
#include "../Bus/BusLineManager.h"

class STNResult {
public:
	GPSPoint point;
	std::string bus_id;
	double dis;
	int dir;
	std::string o_line_id;
	std::string line_id;
	int stauts;
	int row_index;
	int next_station;
	std::string gps_time;
	std::string rec_time;
	time_t gps_time_int;
	double prev_dis;
	double next_dis;

	STNResult(BusData* data) {
		bus_id = data->bus_id;
		o_line_id = data->oline_id;
		point = data->point;
		dir = -1;
		dis = -1;
		line_id = "-";
		row_index = data->index;
		next_station = -1;
		gps_time = data->gps_time;
		rec_time = data->recv_time;
		next_dis = 0;
		prev_dis = 0;

		Time time;
		gps_time_int = time.ToLong(gps_time);
	}

};

class StationJudge {

	int mode = 0;
	int64_t recv_counter = 0;

	BusLineManager* line_manager;

	FILE* fout;

	void PrintResult(std::vector<STNResult*> &result) {

		for (int i = 0; i < result.size(); i++) {
			fprintf(fout, "%d,%s,%s,%d,%d,%.2lf,%.6lf,%.6lf,%s,%s,%s,%.2lf,%.2lf\n",
				result[i]->stauts,
				result[i]->bus_id.c_str(),
				result[i]->line_id.c_str(),
				result[i]->dir,
				result[i]->next_station,
				result[i]->dis,
				result[i]->point.lon,
				result[i]->point.lat,
				result[i]->o_line_id.c_str(),
				result[i]->gps_time.c_str(),
				result[i]->rec_time.c_str(),
				result[i]->prev_dis,
				result[i]->next_dis
			);
		}
	}

	bool CheckSpeed(double dis,time_t delta_time) {
		if (dis / delta_time*3.6 >= 120) return false;
		return true;
	}

	bool JudgeLogic(std::vector<BusData*>& bus, int relation_lineSerial) {

		std::vector<STNResult*> stn_result;

		int start_pos = -1;
		for (int i = 0; i < bus.size(); i++) {
			if (bus[i]->status != -2 && bus[i]->status != -3) {
				pair<GPSPoint*, BusLineUnit*>* unit = line_manager->line_index_judge->GetPoint(bus[i]->point);
				if (unit != NULL) {
					if (unit->second->line_map[relation_lineSerial]) {
						start_pos = i;
						break;
					}
				}
			}
			STNResult* temp_stn = new STNResult(bus[i]);
			if (bus[i]->status == -2 && bus[i]->status == -3) {
				temp_stn->stauts = bus[i]->status;
			}
			else {
				temp_stn->stauts = 0;
			}
			
			stn_result.push_back(temp_stn);
		}

		if (start_pos == -1) return false;

		int dir_num = line_manager->GetLine(relation_lineSerial)->dir_num;
		bool is_Inverting[2];
		is_Inverting[0] = true;
		is_Inverting[1] = true;

		while (start_pos < bus.size()) {

			int right_dir = 0;
			int max_index = 0;
			int min_index = 1000000000;
			int max_pos = 0;

			for (int k = 0; k < dir_num; k++) {
				int _temp_pos = start_pos;
				int _now_index = 0;
				while (_temp_pos<bus.size()) {

					bool flag = false;
					if (bus[_temp_pos]->status != -2 && bus[_temp_pos]->status != -3) {
						pair<GPSPoint*, BusLineUnit*>* unit = line_manager->line_index_judge->GetPoint(bus[_temp_pos]->point);
						if (unit != NULL) {
							for (auto iter = unit->second->line_dir_sets.begin(); iter != unit->second->line_dir_sets.end(); iter++) {
								int line_Id = iter->first/2;
								int dir = iter->first % 2;
								if (line_Id == relation_lineSerial && dir==k) {
									int _min_index=-1;
									for (int i = 0; i < iter->second.size(); i++) {
										if (iter->second[i]->route_serial >= _now_index && (_min_index == -1 || iter->second[i]->route_serial<_min_index)) {
											_min_index = iter->second[i]->route_serial;
										}
									}

									if (_min_index == -1) {
										flag = true;
									}
									else {
										if (_min_index < min_index) min_index = _min_index;
										_now_index = _min_index;
									}
									break;
								}
							}
							if (flag) break;
						}
					}
					_temp_pos++;
				}

				if (_temp_pos > max_pos) {
					max_index = _now_index;
					right_dir = k;
					max_pos = _temp_pos;
				}

			}
			BusLine* line = line_manager->GetLine(relation_lineSerial);

			if (line->sum_routes[right_dir][min_index + 1] <= 500 && line->routes_length[right_dir] - line->sum_routes[right_dir][max_index] <=500 ) {
				is_Inverting[right_dir] = false;
			}
			else {
				//cout<<bus[0]->bus_id<<" " << min_index << " " << max_index << " " << line->routes[right_dir].size() << endl;
			}

			std::vector<STNResult*> temp_queue;

			bool flag = false,un_using=false;
			int _temp_pos = start_pos;
			double prev_dis = 0;

			int error_counter = 0;

			double start_dis = line->routes_length[right_dir], end_dis = 0;
			while (_temp_pos<max_pos) {
				if (bus[_temp_pos]->status != -2 && bus[_temp_pos]->status != -3) {
					pair<GPSPoint*, BusLineUnit*>* unit = line_manager->line_index_judge->GetPoint(bus[_temp_pos]->point);
					if (unit != NULL && unit->second->line_map[relation_lineSerial]) {
						double now_dis = MileagesTools::GetPointMilesOnRoutes(line, right_dir, bus[_temp_pos]->point, prev_dis);
						if (now_dis <= line->routes_length[right_dir]-500) {
							STNResult* temp_stn = new STNResult(bus[_temp_pos]);
							temp_stn->stauts = 1;
							temp_stn->dis = now_dis;
							temp_stn->dir = right_dir;
							temp_stn->line_id = line->line_id;
							temp_stn->next_station = line_manager->GetNextStation(relation_lineSerial, right_dir, now_dis);
							temp_stn->next_dis = line_manager->NextStationDis(relation_lineSerial, right_dir, now_dis, temp_stn->next_station);
							temp_stn->prev_dis = line_manager->PrevStationDis(relation_lineSerial, right_dir, now_dis, temp_stn->next_station - 1);
							temp_queue.push_back(temp_stn);

							if (now_dis <= 500 || now_dis >= line->routes_length[right_dir]-500) {
								temp_stn->stauts = 2;
							}
							else {
								flag = true;
							}
							if (now_dis < start_dis) start_dis = now_dis;
							if (now_dis > end_dis) end_dis = now_dis;

							if (error_counter > 0 && now_dis - prev_dis > 1000) {
								un_using = true;
							}

							error_counter = 0;

							prev_dis = now_dis;
						}
						else {

							if (!flag) {
								STNResult* temp_stn = new STNResult(bus[_temp_pos]);
								temp_stn->stauts = 2;
								temp_queue.push_back(temp_stn);
							}
							else {
								if (now_dis <= line->routes_length[right_dir] - 100 && now_dis >= prev_dis) {
									STNResult* temp_stn = new STNResult(bus[_temp_pos]);
									temp_stn->stauts = 2;
									temp_stn->dis = now_dis;
									temp_stn->dir = right_dir;
									temp_stn->line_id = line->line_id;
									temp_stn->next_station = line_manager->GetNextStation(relation_lineSerial, right_dir, now_dis);
									temp_stn->next_dis = line_manager->NextStationDis(relation_lineSerial, right_dir, now_dis, temp_stn->next_station);
									temp_stn->prev_dis = line_manager->PrevStationDis(relation_lineSerial, right_dir, now_dis, temp_stn->next_station - 1);
									temp_queue.push_back(temp_stn);
									prev_dis = now_dis;
								}
								else {
									break;
								}
							}
						}
					}
					else {
						STNResult* temp_stn = new STNResult(bus[_temp_pos]);
						temp_stn->stauts = 0;
						temp_queue.push_back(temp_stn);
					}

				}
				else {
					STNResult* temp_stn = new STNResult(bus[_temp_pos]);
					temp_stn->stauts = bus[_temp_pos]->status;
					temp_queue.push_back(temp_stn);
				}
				_temp_pos++;
			}

			if (start_dis < end_dis && end_dis - start_dis >= line->routes_length[right_dir] * 0.8 && !un_using) {
				double now_dis = 0;
				time_t now_time;
				for (int i = 0; i < temp_queue.size(); i++) {
					if (now_dis != 0 && !CheckSpeed(temp_queue[i]->dis-now_dis,temp_queue[i]->gps_time_int-now_time)) {
						temp_queue[i]->stauts = 2;
					}
					else {
						if (temp_queue[i]->dis < now_dis) {
							temp_queue[i]->stauts = 2;
						}
						else {
							now_dis = temp_queue[i]->dis;
							now_time = temp_queue[i]->gps_time_int;
						}
					}
					stn_result.push_back(temp_queue[i]);
				}
			}
			else {
				for (int i = 0; i < temp_queue.size(); i++) {
					if (temp_queue[i]->stauts == 1) {
						temp_queue[i]->stauts = 2;
					}
					stn_result.push_back(temp_queue[i]);
				}
			}

			start_pos = _temp_pos;
		}

		bool result = false;
		BusLine* line = line_manager->GetLine(relation_lineSerial);
		for (int k = 0; k < line->dir_num; k++)
			if (is_Inverting[k]) result = true;

		PrintResult(stn_result);

		for (int i = 0; i < stn_result.size(); i++) {
			delete stn_result[i];
		}

		return result;
	}

public:

	StationJudge(BusLineManager* busline_manager) {
		this->line_manager = busline_manager;
		fout = fopen(Configuration::answer_file.c_str(), "w");
	}

	~StationJudge() {
	}

	bool Judge(std::vector<BusData*> &bus,int relation_line) {
		return JudgeLogic(bus, relation_line);
	}

	void Close() {
		fclose(fout);
	}
	
};

#endif