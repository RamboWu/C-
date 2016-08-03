#ifndef RELATION_REPORT_H
#define RELATION_REPORT_H

#include <vector>
#include <map>
#include "../Util/Configuration.h"


class RelationReport {

	std::map<std::string, std::string> single;
	std::map<std::string, std::vector<std::string>> multi;

public:
	RelationReport() {

	}

	void AddSingleRelation(std::string & busId, std::string& lineId) {
		single[busId] = lineId;
	}

	void AddMultiRelation(std::string & busId, std::vector<std::string> & lines) {
		multi[busId] = lines;
	}

	void Report() {
		FILE* fout = fopen(Configuration::single_relation_file_name.c_str(), "w");
		for (auto iter = single.begin(); iter != single.end(); iter++) {

			fprintf(fout, "%s,%s\n", iter->first.c_str(), iter->second.c_str());

		}
		fclose(fout);

		fout = fopen(Configuration::multi_relation_file_name.c_str(), "w");

		for (auto iter = multi.begin(); iter != multi.end(); iter++) {

			fprintf(fout, "%s,[", iter->first.c_str());
			for (int i = 0; i < iter->second.size(); i++) {
				if (i>0) fprintf(fout, "|");
				fprintf(fout, "%s", iter->second[i].c_str());
			}
			fprintf(fout, "]\n");
		}
		fclose(fout);
	}
};
#endif