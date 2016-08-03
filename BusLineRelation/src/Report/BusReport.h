#ifndef BUS_REPORT_H
#define BUS_REPORT_H

#include <vector>
#include <string>
#include "../Util/Configuration.h"

class BusReport {

	FILE* fout;

	std::map<std::string, char*> bus_matching_result;
	/*
	-1:UnKnow
	0:maybe
	1£ºsingle
	2:multi
	3:similar
	*/
	std::map<std::string, int> bus_type;
	std::vector<std::vector<std::string>> similar;

public:

	BusReport() {

	}

	void AddSimilarBus(std::vector<std::string> & bus) {
		similar.push_back(bus);

		for (int i = 0; i < bus.size(); i++) SetBusType(bus[i], 3);
	}

	void AddBusMatchingResult(std::string& busId, char* matching_result) {

		bus_matching_result[busId] = matching_result;

	}

	void SetBusType(std::string& busId, int type) {
		if (bus_type.find(busId) == bus_type.end()) {
			bus_type[busId] = type;
		}
		else {
			bus_type[busId] = type;
		}
	}
	
	void Report() {
		fout = fopen(Configuration::bus_report.c_str(), "w");

		if (Configuration::bus_mind == 1) {
			fprintf(fout, "Similar:");

			for (int i = 0; i < similar.size(); i++) {
				if (i>0) fprintf(fout, ",");
				fprintf(fout, "[");
				for (int j = 0; j < similar[i].size(); j++) {
					if (j>0) fprintf(fout, ",");
					fprintf(fout, "%s", similar[i][j].c_str());
				}
				fprintf(fout, "]");
			}
		}

		fclose(fout);

		fout = fopen(Configuration::matching_rate_file_name.c_str(), "w");
		for (auto iter = bus_matching_result.begin(); iter != bus_matching_result.end(); iter++) {
			int type = -1;
			if (bus_type.find(iter->first) != bus_type.end()) type = bus_type[iter->first];
			fprintf(fout, "%s,%d\n", iter->second, type);
		}
		fclose(fout);


	}
};

#endif