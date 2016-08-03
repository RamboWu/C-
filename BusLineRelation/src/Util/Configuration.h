#pragma once
#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp> 
#include <string>
#include <map>
#include "StringUtils.h"
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



#endif 

