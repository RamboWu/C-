#pragma once
#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp> 
#define MAX_PATH 260

class Configuration {
public:

	static double gps_equal_error;

	static std::string line_data_file;
	static std::string bus_data_file;

	static int index_error;

	static char* config_file;

	static std::string output_dir;
	static std::string answer_file;
	static std::string maybe_relation_file_name;
	static std::string suspicious_relation_file_name;
	static std::string single_relation_file_name;
	static std::string multi_relation_file_name;
	static std::string matching_rate_file_name;

	static std::string temp_bus_file;

	static std::string line_report;
	static std::string bus_report;

	static std::string line_similar_report;
	static std::string line_parent_report;
	static std::string line_front_similar_report;
	static std::string InvertingStationReport;
	static std::string FarAwayStationReport;
	static std::string FormatErrorReport;

	static int bus_mind;
	static int line_mind;
	static int gen_answer;

	static void Load(char* input_file);
	static char* GetFullPath(const char* path);
	static void Init(int num, char* args[]);
};

double Configuration::gps_equal_error = 300;
std::string Configuration::line_data_file;
std::string Configuration::bus_data_file;
int Configuration::index_error = 750;
char* Configuration::config_file;

std::string Configuration::output_dir = "output/";
std::string Configuration::answer_file = "answer.csv";
std::string Configuration::maybe_relation_file_name = "maybe.csv";
std::string Configuration::single_relation_file_name = "single.csv";
std::string Configuration::multi_relation_file_name = "multi.csv";
std::string Configuration::suspicious_relation_file_name = "suspicious.csv";
std::string Configuration::line_similar_report = "LineSimilar.csv";
std::string Configuration::line_parent_report = "LineParent.csv";
std::string Configuration::line_front_similar_report = "LineFrontSimilar.csv";
std::string Configuration::InvertingStationReport = "InvertingStation.csv";
std::string Configuration::FarAwayStationReport = "FarAwayStation.csv";
std::string Configuration::FormatErrorReport = "FormatError.csv";
std::string Configuration::matching_rate_file_name = "BusMatching.csv";

std::string Configuration::line_report = "LineAnalyse.csv";
std::string Configuration::bus_report = "BusAnalyse.csv";

std::string Configuration::temp_bus_file = "tempbus.csv";

int Configuration::line_mind = 1;
int Configuration::bus_mind = 0;
int Configuration::gen_answer = 0;

void Configuration::Init(int num, char* args[]) {
	std::map<std::string, std::string> keyValue;

	for (int i = 0; i < num; i++) {
		std::string temp = args[i];
		std::cout << args[i] << std::endl;
		std::vector<std::string> tags;

		StringUtils::split(tags, temp, "=");
		if (tags.size() == 2) {
			keyValue[tags[0]] = tags[1];
		}
	}

	if (keyValue.find("-l") != keyValue.end()) {
		Configuration::line_data_file = keyValue["-l"];
	}

	if (keyValue.find("-i") != keyValue.end()) {
		Configuration::bus_data_file = keyValue["-i"];
	}

	if (keyValue.find("-a") != keyValue.end()) {
		Configuration::answer_file = keyValue["-a"];
	}

	if (keyValue.find("-o") != keyValue.end()) {
		Configuration::output_dir = keyValue["-o"];
	}

	Configuration::maybe_relation_file_name = Configuration::output_dir + Configuration::maybe_relation_file_name;
	Configuration::single_relation_file_name = Configuration::output_dir + Configuration::single_relation_file_name;
	Configuration::multi_relation_file_name = Configuration::output_dir + Configuration::multi_relation_file_name;
	Configuration::suspicious_relation_file_name = Configuration::output_dir + Configuration::suspicious_relation_file_name;

	Configuration::line_report = Configuration::output_dir + Configuration::line_report;
	Configuration::bus_report = Configuration::output_dir + Configuration::bus_report;
	Configuration::temp_bus_file = Configuration::output_dir + Configuration::temp_bus_file;
	Configuration::line_similar_report = Configuration::output_dir + Configuration::line_similar_report;
	Configuration::line_parent_report = Configuration::output_dir + Configuration::line_parent_report;
	Configuration::line_front_similar_report = Configuration::output_dir + Configuration::line_front_similar_report;
	Configuration::InvertingStationReport = Configuration::output_dir + Configuration::InvertingStationReport;
	Configuration::FormatErrorReport = Configuration::output_dir + Configuration::FormatErrorReport;
	Configuration::FarAwayStationReport = Configuration::output_dir + Configuration::FarAwayStationReport;
	Configuration::matching_rate_file_name = Configuration::output_dir + Configuration::matching_rate_file_name;
}

char* Configuration::GetFullPath(const char* path) {
	char* temp = new char[MAX_PATH];
	memset(temp, 0, sizeof(temp));
	int len = 0;

	for (int i = 0; i < strlen(path); i++) {
		temp[len++] = path[i];
	}
	temp[len] = '\0';
	return temp;
}

void Configuration::Load(char* input_file) {
	config_file = Configuration::GetFullPath(input_file);
	boost::property_tree::ptree pt;
	boost::property_tree::ini_parser::read_ini(config_file, pt);

	if (pt.find("line_data_file") != pt.not_found()) {
		std::string temp_file = pt.get<std::string>("line_data_file");
		Configuration::line_data_file = GetFullPath(temp_file.c_str());
	}
	else {
		std::string temp_file = "input/s_json.csv";
		Configuration::line_data_file = GetFullPath(temp_file.c_str());
	}


	if (pt.find("bus_data_file") != pt.not_found()) {
		std::string temp_bus_file = pt.get<std::string>("bus_data_file");
		Configuration::bus_data_file = GetFullPath(temp_bus_file.c_str());
	}

	if (pt.find("index_error") != pt.not_found()) {
		Configuration::index_error = pt.get<int>("index_error");
	}
	else {
		Configuration::index_error = 750;
	}

	if (pt.find("line_mind") != pt.not_found()) {
		Configuration::line_mind = pt.get<int>("line_mind");
	}
}


#endif 

