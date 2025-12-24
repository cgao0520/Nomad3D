#ifndef CONSTANT_H_97D8ED23_E766_4ac3_98E9_5BD9DB830427
#define CONSTANT_H_97D8ED23_E766_4ac3_98E9_5BD9DB830427

#include <math.h>

namespace Nomad3D
{
#define NM3D_EPSILON_E3			((float)(1E-3))
#define NM3D_EPSILON_E4			((float)(1E-4))
#define NM3D_EPSILON_E5			((float)(1E-5))
#define NM3D_EPSILON_E6			((float)(1E-6))
#define NM3D_PI					((float)3.141592654)

#define NM3D_FIXP16_SHIFT		16
#define NM3D_FIXP16_ROUND_UP	0x00008000 // == 2^15
#define NM3D_FIXP16_ROUND_UP_TO_INT(x)	(((x)+NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT)
#define NM3D_FIXP24_SHIFT		24
#define NM3D_FIXP24_ROUND_UP	0x00800000

#define NM3D_TRI_TYPE_NONE			0x00
#define NM3D_TRI_TYPE_FLAT_TOP		0x01 
#define NM3D_TRI_TYPE_FLAT_BOTTOM	0x02
#define NM3D_TRI_TYPE_FLAT_MASK		0x03 // == NM3D_TRI_TYPE_FLAT_TOP | NM3D_TRI_TYPE_FLAT_BOTTOM
#define NM3D_TRI_TYPE_GENERAL		0x04
#define NM3D_INTERP_LHS				0
#define NM3D_INTERP_RHS				1
//#define NM3D_MAX_VERTICES_PER_POLY	6

#define MAX(a,b)	((a)>(b)?(a):(b))
#define MIN(a,b)	((a)>(b)?(b):(a))
#define SWAP(a,b)	{a=a+b; b=a-b; a=a-b;}
#define FCMP(a,b) ( (fabs(a-b) < NM3D_EPSILON_E3) ? 1 : 0)
#define ABS_MINUS(a,b)	((a)>(b)?(a)-(b):(b)-(a))

#define PIOver180	0.017453292519938f
#define PIUnder180	57.295779513096787f
#define R2D(a)	((a) * PIUnder180)
#define D2R(a)	((a) * PIOver180)
//#define R2D(a)	((a) * 180.0/NM3D_PI)
//#define D2R(a)	((a) * NM3D_PI/180.0)


#define NM3D_JOINT_NO_PARENT	-1

}// namespace Nomad3D

#endif //CONSTANT_H_97D8ED23_E766_4ac3_98E9_5BD9DB830427
