#ifndef LINEREPORT_H
#define LINEREPORT_H

#include <vector>
#include <string>
#include "../Util/Configuration.h"

class LineReport {
	FILE* fout;

	std::vector<std::pair<std::string,std::string>> format_err;
	std::vector<std::vector<std::pair<std::string, std::string>>> similar;
	std::vector<std::pair<std::string, std::string>> not_open;
	std::vector<std::pair<std::string, std::string>> inverting_line;
	std::vector<std::pair<std::string, std::string>> inverting_station;
	std::vector<std::pair<std::string, std::string>> faraway_station;

public:

	void AddFormatError(std::string &lineId,std::string &line_name) {
		format_err.push_back(std::pair<std::string,std::string>(lineId,line_name));
	}

	void AddNotOpen(std::string &lineId, std::string &line_name) {
		not_open.push_back(std::pair<std::string, std::string>(lineId, line_name));
	}

	void AddSimilarLine(std::vector<std::pair<std::string, std::string>> & lines) {
		similar.push_back(lines);
	}

	void AddInvertingLine(std::string &lineId, std::string &line_name) {
		inverting_line.push_back(std::pair<std::string, std::string>(lineId, line_name));
	}

	void AddInvertingStation(std::string &lineId, std::string &line_name) {
		inverting_station.push_back(std::pair<std::string, std::string>(lineId, line_name));
	}

	void AddFarAwayStation(std::string &lineId, std::string &line_name, std::string &describe) {
		faraway_station.push_back(std::pair<std::string, std::string>(lineId, line_name+","+describe));
	}

	void Report() {
		fout = fopen(Configuration::line_report.c_str(), "w");

		FILE* temp_out = fopen(Configuration::FormatErrorReport.c_str(), "w");
		fprintf(fout,"FormatErr:");
		for (int i = 0; i < format_err.size(); i++) {
			if (i>0) fprintf(fout, ",");
			fprintf(fout, "%s", format_err[i].first.c_str());
			fprintf(temp_out, "%s,%s\n", format_err[i].first.c_str(), format_err[i].second.c_str());
		}
		fclose(temp_out);
		fprintf(fout, "\n");

		fprintf(fout, "InvertingLine:");
		for (int i = 0; i < inverting_line.size(); i++) {
			if (i>0) fprintf(fout, ",");
			fprintf(fout, "%s", inverting_line[i].first.c_str());
		}
		fprintf(fout, "\n");

		temp_out = fopen(Configuration::InvertingStationReport.c_str(), "w");
		fprintf(fout, "InvertingStation:");
		for (int i = 0; i < inverting_station.size(); i++) {
			if (i>0) fprintf(fout, ",");
			fprintf(fout, "%s", inverting_station[i].first.c_str());
			fprintf(temp_out, "%s,%s\n", inverting_station[i].first.c_str(), inverting_station[i].second.c_str());
		}
		fclose(temp_out);
		fprintf(fout, "\n");

		temp_out = fopen(Configuration::FarAwayStationReport.c_str(), "w");
		fprintf(fout, "FarAwayStation:");
		for (int i = 0; i < faraway_station.size(); i++) {
			if (i>0) fprintf(fout, ",");
			fprintf(fout, "%s", faraway_station[i].first.c_str());
			fprintf(temp_out, "%s,%s\n", faraway_station[i].first.c_str(), faraway_station[i].second.c_str());
		}
		fclose(temp_out);
		fprintf(fout, "\n");

		fprintf(fout, "NotOpen:");
		for (int i = 0; i < not_open.size(); i++) {
			if (i>0) fprintf(fout, ",");
			fprintf(fout, "%s", not_open[i].first.c_str());
		}
		fprintf(fout, "\n");

		temp_out = fopen(Configuration::line_similar_report.c_str(), "w");
		fprintf(fout, "Similars:");
		for (int i = 0; i < similar.size(); i++) {
		
			if (i>0) fprintf(fout, ",");
			fprintf(fout, "[");
			for (int j = 0; j < similar[i].size(); j++) {
				if (j > 0) {
					fprintf(fout, ",");
					fprintf(temp_out, ",");
				}
				fprintf(fout, "%s", similar[i][j].first.c_str());
				fprintf(temp_out, "%s,%s", similar[i][j].first.c_str(), similar[i][j].second.c_str());
			}
			fprintf(fout, "]");
			fprintf(temp_out, "\n");
		}
		fprintf(fout, "\n");
		fclose(temp_out);

		fclose(fout);
	}

};
#endif