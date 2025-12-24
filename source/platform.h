
#ifndef PLATFORM_H_D7F9C56E_6233_450b_9BEA_74C6DBA8F659
#define PLATFORM_H_D7F9C56E_6233_450b_9BEA_74C6DBA8F659

#include "config.h"
#include <cstddef>

#if defined(NM3D_PLATFORM_WIN32)

#include <windows.h>

 #pragma  warning( disable : 4244)	//disable several warnings
// #pragma  warning( disable : 4305)
// #pragma  warning( disable : 4018)
// #pragma  warning( disable : 4267)
// #pragma  warning( disable : 4101)
// #pragma  warning( disable : 4002)
// #pragma  warning( disable : 4996)

#elif defined(NM3D_PLATFORM_NDS)

#include "nds.h"

#define UINT		uint32
#define SHORT		s16
#define INT			s32
#define	USHORT		uint16
#define UCHAR		uint8
#define CHAR		u8
#define BYTE		unsigned char
#define LONG		long
#define DWORD		unsigned long
#define WORD		unsigned short
#define FAR

#elif defined(NM3D_PLATFORM_LINUX)

#include <stdint.h>

#define UINT		uint32_t
#define SHORT		int16_t
#define INT			int32_t
#define	USHORT		uint16_t
#define UCHAR		uint8_t
#define CHAR		int8_t
#define BYTE		unsigned char
#define LONG		long
#define DWORD		uint32_t
#define WORD		uint16_t
#define FAR

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#endif //NM3D_PLATFORM_XXX

#if !defined(BIT)
#define  BIT(x)	1<<(x)
#endif

#if defined(NM3D_PIXELFORMAT_555) || defined(NM3D_PIXELFORMAT_565)
#define NM3D_BUFFER	unsigned short
#else
#define NM3D_BUFFER unsigned int
#endif

#endif // PLATFORM_H_D7F9C56E_6233_450b_9BEA_74C6DBA8F659
