#ifndef BASETYPE_H_83708AAA_5E0C_44b3_93B6_927C804E3F1C
#define BASETYPE_H_83708AAA_5E0C_44b3_93B6_927C804E3F1C

#include "platform.h"

#if defined(NM3D_PLATFORM_WIN32) || defined(NM3D_PLATFORM_LINUX)
	typedef	float n3dFloat;
#elif defined(NM3D_PLATFORM_NDS)
	typedef float n3dFloat;
#elif defined(NM3D_PLATFORM_WIN32_64) || defined(NM3D_PLATFORM_LINUX_64)
	typedef double n3dFloat;
#endif

#endif //BASETYPE_H_83708AAA_5E0C_44b3_93B6_927C804E3F1C
