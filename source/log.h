// log.h: interface for the CLog class.
//
//////////////////////////////////////////////////////////////////////

#ifndef LOG_H__41FBCEF8_DA41_4336_932E_1915343B6217
#define LOG_H__41FBCEF8_DA41_4336_932E_1915343B6217

#include "config.h"
#include "constant.h"
#include "platform.h"

class CLog  
{
public:
	CLog();
	CLog(const char * szFilename);
	virtual ~CLog();
public:
	bool Init(const char * szFile);
	void Write(const char * szColor, const char * szFmt, ...);
protected:
	char * m_szFilename;
};

#if NM3D_DEBUG_ON
	extern CLog g_log;
	#define NM3D_DEBUG_COLOR_RED	"#ff0000"
	#define NM3D_DEBUG_COLOR_GREEN	"#00ff00"
	#define NM3D_DEBUG_COLOR_BLUE	"#0000ff"
	#define NM3D_DEBUG_COLOR_WHITE	"#ffffff"
	#define NM3D_DEBUG_COLOR_YELLOW	"#ffff00"
	//#define NM3D_DEBUG_PRINT(szColor,fmt,args...) g_log.Write(szColor, format, ##args); //GCC style
	//#define NM3D_DEBUG_PRINT(szColor, fmt, ...)	g_log.Write(szColor, fmt, __VA_ARGS__); //C99 style
	#define NM3D_DEBUG_PRINT		g_log.Write
	#define NM3D_DEBUG_PRINT_MATRIX(clr,m)	for(int i=0; i<4; i++) \
												NM3D_DEBUG_PRINT(clr,"%f, %f, %f, %f",m[i][0],m[i][1],m[i][2],m[i][3]);
#else
	#define NM3D_DEBUG_PRINT//(szColor, fmt, ...) 
#endif

#endif //LOG_H__41FBCEF8_DA41_4336_932E_1915343B6217
