#ifndef _DUMPGENERATION_H_
#define _DUMPGENERATION_H_

#include <string>

#ifdef _WIN32
#pragma comment(lib, "dbghelp.lib")
#endif

namespace Utility
{
	void DumpGenerationInit(std::string app_name);
}

#endif