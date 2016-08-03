#include "DumpGeneration.h"
#include <Windows.h>
#include <dbghelp.h>   
#include <shellapi.h>   
#include <shlobj.h>
#include <stdio.h>
#include <string>


namespace Utility
{

std::string g_AppName = "";

LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS *pExceptionPointers)   
{   
	SetErrorMode( SEM_NOGPFAULTERRORBOX );   
  
	//收集信息   
	std::string strBuild;   
	strBuild = "Build: ";
	strBuild = strBuild + __DATE__ + __TIME__;
  

	HMODULE hModule;   
	char szModuleName[MAX_PATH] = "";   
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)pExceptionPointers->ExceptionRecord->ExceptionAddress, &hModule);   
	GetModuleFileNameA(hModule, szModuleName, ARRAYSIZE(szModuleName));   

	   std::string strError;   
	char error_str[512];
	sprintf_s(error_str, 512, "%s %d , %d ,%d.", szModuleName,pExceptionPointers->ExceptionRecord->ExceptionCode, pExceptionPointers->ExceptionRecord->ExceptionFlags, pExceptionPointers->ExceptionRecord->ExceptionAddress);  
	strError = error_str;
  
	//生成 mini crash dump   
	BOOL bMiniDumpSuccessful;   
	//char szPath[MAX_PATH];    
	char szFileName[MAX_PATH];    
	//char* szAppName = "AutoMatchServer";   
	char* szAppName =   (char*)g_AppName.c_str();
	//char* szVersion = "v1.0";   
	DWORD dwBufferSize = MAX_PATH;   
	HANDLE hDumpFile;   
	SYSTEMTIME stLocalTime;   
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;   
	GetLocalTime( &stLocalTime );   

	//GetTempPath( dwBufferSize, szPath );   
	//sprintf_s( szFileName, MAX_PATH, "%s%s", szPath, szAppName );   
	sprintf_s(szFileName, MAX_PATH, "%s",  szAppName);
	//CreateDirectory( szFileName, NULL );   
	sprintf_s( szFileName, MAX_PATH, "%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",    
			   /*szPath, */szAppName, /*szVersion,    */
			   stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,    
			   stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,    
			   GetCurrentProcessId(), GetCurrentThreadId());   

	hDumpFile = CreateFileA(szFileName, 
							GENERIC_WRITE,    
							0, 
							0, 
							CREATE_NEW, 
							FILE_ATTRIBUTE_NORMAL, 
							0);   


	//hDumpFile = CreateFile(szFileName, GENERIC_READ|GENERIC_WRITE,    
	  //          FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);   

	char log[201];
	sprintf_s(log, 200, "hDumpFile %d", hDumpFile);

  
	MINIDUMP_USER_STREAM UserStream[2];   
	MINIDUMP_USER_STREAM_INFORMATION UserInfo;   
	UserInfo.UserStreamCount = 1;   
	UserInfo.UserStreamArray = UserStream;   
	UserStream[0].Type = CommentStreamW;   
	UserStream[0].BufferSize = strBuild.size();
	UserStream[0].Buffer = (void*)strBuild.data();
	UserStream[1].Type = CommentStreamW;   
	UserStream[1].BufferSize = strError.size();
	UserStream[1].Buffer = (void*)strError.data();
  
	ExpParam.ThreadId = GetCurrentThreadId();   
	ExpParam.ExceptionPointers = pExceptionPointers;   
	ExpParam.ClientPointers = TRUE;   
	   
	MINIDUMP_TYPE MiniDumpWithDataSegs = (MINIDUMP_TYPE)(MiniDumpNormal    
			| MiniDumpWithHandleData    
			| MiniDumpWithUnloadedModules    
			| MiniDumpWithIndirectlyReferencedMemory    
			| MiniDumpScanMemory    
			| MiniDumpWithProcessThreadData    
			| MiniDumpWithThreadInfo);   
	bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),    
					hDumpFile, MiniDumpNormal, &ExpParam, &UserInfo, NULL); 

	int e = GetLastError();

	sprintf_s(log, 200, "PHANDLE %d PID %d MiniDumpWriteDump %d error code %d", 
			GetCurrentProcess(), GetCurrentProcessId(), bMiniDumpSuccessful, e);

  // 上传mini dump 到自己服务器（略）   
  //...   
  
  return EXCEPTION_CONTINUE_SEARCH; //或者 EXCEPTION_EXECUTE_HANDLER 关闭程序   
}   



void DumpGenerationInit(std::string app_name)
{
	g_AppName = app_name;
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter); 
}

}