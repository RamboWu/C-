
#include "Relation/RelationGenerator.h"
#include "Relation/DirScanMatching.h"
#include "MiniDump/DumpGeneration.h"

int main(int argc, char* argv[]) {

	Utility::DumpGenerationInit("Matching");

	if (strcmp(argv[1], "-A") == 0) {
		Configuration::Init(argc, argv);
		//Configuration::Load("input/config.ini");

		RelationGenerator* relation_gen = new RelationGenerator;
		relation_gen->Init();
		cout << "Init Completed" << endl;
		relation_gen->CalcRelation(Configuration::bus_data_file.c_str());
		cout << "Calc Completed" << endl;
		relation_gen->Close();
	}

	//-M D:\carno_gps\zy_gps\A\ input/s_json.csv HangZhouAir/Matching.csv
	if (strcmp(argv[1], "-M") == 0) {
		DirScanMatching matching(argv[3],argv[4],argv[5]);
		matching.DirScan(argv[2]);
		matching.Close();
	}

	if (strcmp(argv[1], "-R") == 0) {
		cout << "1" << endl;
		DirScanMatching matching(argv[3], argv[4], argv[5]);
		matching.FileMatching(argv[2]);
		matching.Close();
	}

}