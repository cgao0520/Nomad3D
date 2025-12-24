// log.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "log.h"
#include "config.h"
#include <stdio.h>
#include <cstdarg>

#if defined(NM3D_PLATFORM_WIN32)
#include <ctime>
#elif defined(NM3D_PLATFORM_LINUX)
#include <time.h>
#else
#error "This platform doesn't support log!"
#endif

#if NM3D_DEBUG_ON
	CLog g_log("log.html");
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLog::CLog()
{

}

CLog::CLog(const char * szFilename)
{
	Init(szFilename);
}


CLog::~CLog()
{

}

bool CLog::Init(const char * szFile)
{
	char caTime[32];
	char caDate[32];
	FILE * f = fopen(szFile, "wt");
	if(!f)
		return false;
	
	fprintf(f, "<HTML>\n<HEAD><TITLE>Nomad3D Log</TITLE></HEAD>\n<BODY BGCOLOR = \"#000000\">\n");

#if defined(NM3D_PLATFORM_WIN32)
    _strtime(caTime);
    _strdate(caDate);
#elif defined(NM3D_PLATFORM_LINUX)
	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	strftime(caTime, sizeof(caTime), "%H:%M:%S", tm_info);
	strftime(caDate, sizeof(caDate), "%m/%d/%y", tm_info);
#else
	caTime=caDate="NoTime";
#endif
	fprintf(f, "<FONT COLOR = \"#FFFFFF\">Log Started at %s on %s</FONT><BR><BR>\n", caTime, caDate);
	fprintf(f, "</BODY></HTML>");
	fclose(f);
	m_szFilename = (char *)szFile;
	
	return true;
}

void CLog::Write(const char * szColor, const char * szFmt, ...)
{
	char caString[1024];
	char caTime[32];
	
	va_list va;
	va_start(va, szFmt);
	vsprintf(caString, szFmt, va);
	va_end(va);
#if defined(NM3D_PLATFORM_WIN32)
	_strtime(caTime);
#elif defined(NM3D_PLATFORM_LINUX)
	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	strftime(caTime, sizeof(caTime), "%H:%M:%S", tm_info);
#else
	caTime="NoTime";
#endif
	
	FILE * f = fopen(m_szFilename, "r+");
	if(!f)
		return;
	
	fseek(f, -14, SEEK_END);
	fprintf(f, "<FONT COLOR = \"%s\">%s&nbsp;&nbsp;&nbsp;&nbsp;%s</FONT><BR>\n", szColor, caTime, caString);
	fprintf(f, "</BODY></HTML>");
	fclose(f);
}