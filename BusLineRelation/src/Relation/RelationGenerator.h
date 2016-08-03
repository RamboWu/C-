#ifndef RELATION_GENERATOR_H
#define RELATION_GENERATOR_H

#include<iomanip>
#include <boost/filesystem.hpp>
#include "../Bus/BusLineManager.h"
#include "../Util/Configuration.h"
#include "../Index/PlaneIndex.h"
#include "../Bus/BusLineIndexInfo.h"
#include "../Util/MileagesTools.h"
#include "../Bus/Bus.h"
#include "../Report/LineReport.h"
#include "../Report/BusReport.h"
#include "../Judge/StationJudge.h"
#include "LineSimilar.h"
#include "BusSimilar.h"
#include "RoutesMatching.h"
#include "StationMatching.h"
#include "MatchingResult.h"
#include "LineFrontSimilar.h"

class BusLineRelation{
	std::string busId;
	std::vector<std::string> lineId;
};

class LineDir {
public:
	std::string line;
	int dir;
	int nextStation;
	LineDir(std::string & line, int dir, int nextStation) {
		this->line = line;
		this->dir = dir;
		this->nextStation = nextStation;
	}
};

class LineDirCompare {
public:
	bool operator() (const LineDir& a, const LineDir& b) {
		if (a.line < b.line) return true;
		if (a.line > b.line) return false;
		if (a.dir < b.dir) return true;
		if (a.dir > b.dir) return false;
		if (a.nextStation < b.nextStation) return true;
		return false;
	}
};

class RelationGenerator {

	std::map<std::string, BusLineRelation*> relation;

	std::set<int> match_lines;
	std::set<int> miss_lines;
	std::set<std::string> miss_bus;

	std::set<std::string> inverting_Lines;

	BusLineManager* line_manager;
	RoutesMatching* routes_matching;
	StationMatching* station_matching;
	StationJudge* station_judge;

	FILE* maybe_out;
	FILE* single_relation;
	FILE* mutil_relation;
	FILE* suspicious_relation;

	FILE* temp_bus_file;

	LineReport* line_report;
	BusReport* bus_report;
	LineSimilar* line_similar;
	LineFrontSimilar* line_front_similar;
	BusSimilar* bus_similar;

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


	void GetRelation(std::string & busId,std::vector<BusData*>& bus) {

		MatchingResult* match_result = new MatchingResult;
		routes_matching->GetMatchingRate(bus, match_result,2);
		station_matching->GetMatchingRate(bus, match_result);

		std::set<int> judgeLines;
		for (int i = 0; i < bus.size(); i++) {
			if (bus[i]->status == 1 && bus[i]->bus_status == 1 && bus[i]->is_error == 1) {
				int serial = line_manager->GetSerialByLineId(bus[i]->line_id);

				if (serial != -1) {
					judgeLines.insert(serial);
				}
				
			}
		}

		std::set<LineDir, LineDirCompare> stnMap;
		std::map<LineDir, int, LineDirCompare> stnCounter;
		std::map<std::string, int> tuningCounter;
		for (int i = 0; i < bus.size(); i++) {
			BusData* temp = bus[i];

			if (temp->status == 1 && temp->bus_status == 1 && temp->is_error == 1) {
				std::string bus_dir, bus_nextstation;
				StringUtils::ToString(temp->dir, bus_dir);
				StringUtils::ToString(temp->next_station, bus_nextstation);

				LineDir temp_LineDir = LineDir(temp->line_id, temp->dir, temp->next_station);
				LineDir temp_LineDir0 = LineDir(temp->line_id, temp->dir, 0);

				if (temp->next_station > 1) {
					if (stnMap.find(temp_LineDir) == stnMap.end()) {
						stnMap.insert(temp_LineDir);
						if (stnCounter.find(temp_LineDir0) == stnCounter.end())
							stnCounter[temp_LineDir0] = 0;

						stnCounter[temp_LineDir0]++;
					}
					if (tuningCounter.find(temp->line_id) == tuningCounter.end())
						tuningCounter[temp->line_id] = 0;
					tuningCounter[temp->line_id] |= 1 << temp->dir;
				}
			}
		}

		int max_bus_to_line = -1;
		int max_line_to_bus = -1;
		int max_station_to_bus = -1;

		std::set<int> max_bus_to_line_total;
		std::set<int> max_line_to_bus_total;
		std::set<int> max_station_to_bus_total;

		for (auto iter = match_result->bus_to_line_num.begin(); iter != match_result->bus_to_line_num.end(); iter++) {
			if (max_bus_to_line==-1 || iter->second > match_result->bus_to_line_num[max_bus_to_line]) {
				max_bus_to_line = iter->first;
				max_bus_to_line_total.clear();
				max_bus_to_line_total.insert(iter->first / 2);
			}
			else if (max_bus_to_line != -1 && iter->second == match_result->bus_to_line_num[max_bus_to_line] && max_bus_to_line_total.find(iter->first /2)==max_bus_to_line_total.end()) {
				max_bus_to_line_total.insert(iter->first / 2);
			}
		}

		for (auto iter = match_result->line_to_bus_num.begin(); iter != match_result->line_to_bus_num.end(); iter++) {
			if (max_line_to_bus == -1 || iter->second > match_result->line_to_bus_num[max_line_to_bus]) {
				max_line_to_bus = iter->first;
				max_line_to_bus_total.clear();
				max_line_to_bus_total.insert(iter->first / 2);
			}
			else if (max_line_to_bus != -1 && iter->second == match_result->line_to_bus_num[max_line_to_bus] && max_line_to_bus_total.find(iter->first/2)==max_line_to_bus_total.end()) {
				max_line_to_bus_total.insert(iter->first / 2);
			}
		}

		for (auto iter = match_result->station_to_bus_num.begin(); iter != match_result->station_to_bus_num.end(); iter++) {
			if (max_station_to_bus == -1 || iter->second > match_result->station_to_bus_num[max_station_to_bus]) {
				max_station_to_bus = iter->first;
				max_station_to_bus_total.clear();
				max_station_to_bus_total.insert(iter->first / 2);
			}
			else if (max_station_to_bus != -1 && iter->second == match_result->station_to_bus_num[max_station_to_bus] && max_station_to_bus_total.find(iter->first/2)==max_station_to_bus_total.end()) {
				max_station_to_bus_total.insert(iter->first / 2);
			}
		}

		std::string max_bus_to_line_str = GetLineSetString(max_bus_to_line_total);
		std::string max_line_to_bus_str = GetLineSetString(max_line_to_bus_total);
		std::string max_station_to_bus_str = GetLineSetString(max_station_to_bus_total);

		char* temp_match_str = new char[max_bus_to_line_str.length() + max_line_to_bus_str.length() + max_station_to_bus_str.length()+100];
		sprintf_s(temp_match_str, max_bus_to_line_str.length() + max_line_to_bus_str.length() + max_station_to_bus_str.length() + 100,
				"%s,%s,%.2lf,%d,%s,%.2lf,%d,%s,%.2lf,%d",
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
		bus_report->AddBusMatchingResult(busId, temp_match_str);

		std::set<std::string> correctLine;
		for (auto iter = stnCounter.begin(); iter != stnCounter.end(); iter++)
			if (line_manager->GetLine(iter->first.line)!= NULL) {

				BusLine* line = line_manager->GetLine(iter->first.line);

				int line_serial = line_manager->GetSerialByLineId(iter->first.line);

				if (line->dir_num == 2) {

					for (int k = 0; k < line->dir_num; k++) {
						double bus_to_line = match_result->bus_to_line[line_serial * 2 + k];
						double line_to_bus = match_result->line_to_bus[line_serial * 2 + k];
						double station = match_result->station_to_bus[line_serial * 2 + k];

						if (tuningCounter.find(iter->first.line) != tuningCounter.end() &&
							tuningCounter[iter->first.line] == 3 &&
							iter->second >= line_manager->GetLine(iter->first.line)->station_num[iter->first.dir] / 2
							) {

							if (bus_to_line >= 0.9 && line_to_bus >= 0.9) {
								if (correctLine.find(iter->first.line) == correctLine.end()) {
									correctLine.insert(iter->first.line);
								}
							}
							else {
								if ((line_to_bus >= 0.9|| bus_to_line>=0.9) && station >= 0.95) {
									if (correctLine.find(iter->first.line) == correctLine.end()) {
										correctLine.insert(iter->first.line);
									}
								}
							}
							
						}
					}
				}
				else {
					double bus_to_line = match_result->bus_to_line[line_serial * 2];
					double line_to_bus = match_result->line_to_bus[line_serial * 2];
					double station = match_result->station_to_bus[line_serial * 2];

					if (iter->second >= line_manager->GetLine(iter->first.line)->station_num[iter->first.dir] / 2) {

						if (bus_to_line >= 0.9 && line_to_bus >= 0.9) {
							if (correctLine.find(iter->first.line) == correctLine.end()) {
								correctLine.insert(iter->first.line);
							}
						}
						else {
							if ((line_to_bus >= 0.9 || bus_to_line >= 0.9) && station >= 0.95) {
								if (correctLine.find(iter->first.line) == correctLine.end()) {
									correctLine.insert(iter->first.line);
								}
							}
						}
					}
				}
			}

		switch (correctLine.size()) {
			case 0:{
				fprintf(maybe_out, "%s,", busId.c_str());
				std::set<std::string> temp;
				int counter = 0;
				for (auto iter = match_result->bus_to_line.begin(); iter != match_result->bus_to_line.end(); iter++) {
					double bus_to_line = iter->second;
					double line_to_bus = match_result->line_to_bus[iter->first];
					double station = match_result->station_to_bus[iter->first];
					BusLine* line = line_manager->GetLine(iter->first >> 1);
					if ((bus_to_line > 0.5 && line_to_bus > 0.8 && station>0.8) && temp.find(line->line_id)==temp.end()) {
						temp.insert(line->line_id);
						if (counter>0) fprintf(maybe_out, "|");
						counter++;
						miss_lines.insert(line->serial);
						miss_bus.insert(busId);
						fprintf(maybe_out, "%s(%.2lf,%.2lf,%.2lf)", line->line_id.c_str(), bus_to_line, line_to_bus, station);
						bus_report->SetBusType(busId, 0);
					}
				}

				fprintf(maybe_out, "\n");

				if (Configuration::bus_mind == 1) {
					if (counter == 0) {
						bus_similar->InsertBus(bus);
						for (int i = 0; i < bus.size(); i++) {
							bus[i]->ToOutput(temp_bus_file);
						}
					}
				}

				break;
			}
			case 1:{
				BusLine* line = line_manager->GetLine(*(correctLine.begin()));
				if (line != NULL) {

					if (max_bus_to_line / 2  == line->serial && max_line_to_bus /2 == line->serial) {
						
						if (Configuration::gen_answer == 1) {
							station_judge->Judge(bus, line->serial);
						}
						
						bool is_Inverting = false;

						if (!is_Inverting) {
							fprintf(single_relation, "%s,%s\n", busId.c_str(), line->line_id.c_str());
							match_lines.insert(line->serial);
							bus_report->SetBusType(busId, 1);
						}
						else {
							if (inverting_Lines.find(line->line_id) == inverting_Lines.end()) {
								inverting_Lines.insert(line->line_id);
								line_report->AddInvertingLine(line->line_id,line->line_name);
							}
						}
					}
					else {
						BusLine* temp_line = line_manager->GetLine(max_bus_to_line /2 );
						fprintf(suspicious_relation, "%s,%s,%s\n", busId.c_str(),line->line_id.c_str(),temp_line->line_id.c_str());
					}
				}
				break;
			}
			default:{
				fprintf(mutil_relation, "%s,",busId.c_str());
				for (auto iter = correctLine.begin(); iter != correctLine.end(); iter++) {
					BusLine* line = line_manager->GetLine(*iter);
					if (iter != correctLine.begin()) fprintf(mutil_relation, "|");
					fprintf(mutil_relation, "%s", line->line_id.c_str());
				}
				fprintf(mutil_relation, "\n");
				bus_report->SetBusType(busId, 2);
				break;
			}
		}

		delete match_result;
	}

public:

	RelationGenerator() {
		line_report = new LineReport;
		bus_report = new BusReport;
		line_manager = new BusLineManager;
		line_similar = new LineSimilar(line_report);
		line_front_similar = new LineFrontSimilar();
		
		if (Configuration::gen_answer == 1) {
			station_judge = new StationJudge(line_manager);
		}

		maybe_out = fopen(Configuration::maybe_relation_file_name.c_str(), "w");
		single_relation = fopen(Configuration::single_relation_file_name.c_str(), "w");
		mutil_relation = fopen(Configuration::multi_relation_file_name.c_str(), "w");
		suspicious_relation = fopen(Configuration::suspicious_relation_file_name.c_str(), "w");

		temp_bus_file = fopen(Configuration::temp_bus_file.c_str(), "w");
	}

	~RelationGenerator() {
		delete routes_matching;
		delete station_matching;
		delete line_manager;
		delete line_similar;
		delete line_front_similar;
	}

	void Init() {
		line_manager->Load(Configuration::line_data_file.c_str(),line_report);
		routes_matching = new RoutesMatching(line_manager);
		station_matching = new StationMatching(line_manager);

		bus_similar = new BusSimilar(bus_report, line_manager);

		line_similar->CalcSimilar(line_manager);
		line_front_similar->CalcSimilar(line_manager);
	}

	void CalcRelation(const char* input) {
		std::vector<BusData*> bus;

		std::ifstream in(input);
		std::string temp;

		std::string prev_busId = "";

		int total = 0;

		while (std::getline(in, temp)) {
			total++;
			if (total % 100000 == 0) {
//				cout << total << endl;
			}
			BusData* data = new BusData;
			data->parse(temp);

			if (prev_busId == "" || prev_busId != data->bus_id) {
				if (prev_busId != "") {
					GetRelation(prev_busId,bus);
				}
				prev_busId = data->bus_id;
				for (int i = 0; i < bus.size(); i++) delete bus[i];
				bus.clear();
			}
			bus.push_back(data);
		}

		if (bus.size()>0) {
			GetRelation(prev_busId,bus);
			for (int i = 0; i < bus.size(); i++) delete bus[i];
			bus.clear();
		}

		in.close();
	}


	void CalcSimilarBus() {
		std::ifstream in(Configuration::temp_bus_file);
		std::string temp;
		std::string prev_busId = "";

		std::vector<BusData*> bus;

		while (std::getline(in, temp)) {
			BusData* data = new BusData;
			data->parse(temp);

			if (prev_busId == "" || prev_busId != data->bus_id) {
				if (prev_busId != "") {
					bus_similar->CalcSimilar(bus);
				}
				prev_busId = data->bus_id;
				for (int i = 0; i < bus.size(); i++) delete bus[i];
				bus.clear();
			}
			bus.push_back(data);
		}

		if (bus.size()>0) {
			bus_similar->CalcSimilar(bus);
			for (int i = 0; i < bus.size(); i++) delete bus[i];
			bus.clear();
		}

		in.close();
	}

	void InsertSimilarBus() {
		std::ifstream in(Configuration::temp_bus_file);
		std::string temp;
		std::string prev_busId = "";

		std::vector<BusData*> bus;

		while (std::getline(in, temp)) {
			BusData* data = new BusData;
			data->parse(temp);

			if (prev_busId == "" || prev_busId != data->bus_id) {
				if (prev_busId != "") {
					bus_similar->InsertBus(bus);
				}
				prev_busId = data->bus_id;
				for (int i = 0; i < bus.size(); i++) delete bus[i];
				bus.clear();
			}
			bus.push_back(data);
		}

		if (bus.size()>0) {
			bus_similar->InsertBus(bus);
			for (int i = 0; i < bus.size(); i++) delete bus[i];
			bus.clear();
		}

		in.close();
	}

	void Close() {

		std::vector<int> deletes;

		for (auto iter = miss_lines.begin(); iter != miss_lines.end(); iter++) {
			if (match_lines.find(*iter) != match_lines.end()) {
				deletes.push_back(*iter);
			}
		}

		for (int i = 0; i < deletes.size(); i++) {
			miss_lines.erase(deletes[i]);
		}

		for (int i = 0; i < line_manager->line_total; i++) {
			BusLine* line = line_manager->GetLineByNum(i);
			if (match_lines.find(line->serial) == match_lines.end() && miss_lines.find(line->serial)==miss_lines.end()) {
				line_report->AddNotOpen(line->line_id,line->line_name);
			}
		}

		cout << "AddNotOpen" << endl;

		fclose(maybe_out);
		fclose(single_relation);
		fclose(mutil_relation);
		fclose(suspicious_relation);
		fclose(temp_bus_file);

		cout << "Close Completed" << endl;

		line_report->Report();

		cout << "Line Report" << endl;
		if (Configuration::bus_mind == 1) {
			CalcSimilarBus();
			bus_similar->Summary();
		}

		bus_report->Report();

		cout << "Bus Report" << endl;

		if (Configuration::gen_answer == 1) {
			station_judge->Close();
			cout << "Station Judge" << endl;
		}
		
	}

};

#endif