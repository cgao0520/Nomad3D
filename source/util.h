#ifndef UTIL_H_7708765B_8514_4236_99FE_B8922E68514E
#define UTIL_H_7708765B_8514_4236_99FE_B8922E68514E

#include <math.h>
#include <cassert>
#include "constant.h"
#include "lut.h"

namespace Nomad3D
{
	
	inline float Sqr(float f)
	{
		return f * f;
	}
	
	inline float Q_rsqrt( float number ) 
	{ 
		//long i;
		int i;
		float x2, y;
		const float threehalfs = 1.5F;
		x2 = number * 0.5F;
		y  = number;
		i  = * ( /*long*/int * ) & y;
		i  = 0x5f3759df - ( i >> 1 );
		y  = * ( float * ) &i;
		y  = y * ( threehalfs - ( x2 * y * y ) );
		//y  = y * ( threehalfs - ( x2 * y * y ) );
#ifndef Q3_VM 
#ifdef __linux__ 
		assert( !isnan(y) ); // bk010122 - FPE? 
#endif 
#endif 
		return y; 
	} 
	
	inline float InvSqrt2 (float x)
	{
		union
		{
			int intPart;
			float floatPart;
		} convertor;
		
		convertor.floatPart = x;
		convertor.intPart = 0x5f3759df - (convertor.intPart >> 1);
		return 0.5f*convertor.floatPart*(3.0f - x*convertor.floatPart*convertor.floatPart);
	}
	/*
	inline float mysqrt(float x)
	{
	union
	{
	int intPart;
	float floatPart;
	} convertor;
	union
	{
	int intPart;
	float floatPart;
	} convertor2;
	
	  convertor.floatPart = x;
	  convertor2.floatPart = x;
	  convertor.intPart = 0x1FBCF800 + (convertor.intPart >> 1);
	  convertor2.intPart = 0x5f3759df - (convertor2.intPart >> 1);
	  return 0.5f*(convertor.floatPart + (x * convertor2.floatPart));
	  }
	*/
	inline float Sqrt(float x)
	{
		return (float)1.0/Q_rsqrt(x);
	}
	
	inline float Abs(float f)
	{
		return (f >= 0) ? f : -f;
	}
	
	inline bool IsZero(float f)
	{
		return (Abs(f) < NM3D_EPSILON_E5);
	}
	
	inline bool FuzEQ(float f, float g)
	{
		return (f <= g) ? (f >= g - NM3D_EPSILON_E5) : (f <= g + NM3D_EPSILON_E5);
	}

	inline float Lerp(float g, float h, float f)
	{
		return g + (h - g) * f;
	}
	
	unsigned char Log2(int x);

	float Sin(float angle);
	float SinR(float angle);
	float Cos(float angle);
	float CosR(float angle);
	float Tan(float angle);
	
} // namespace Nomad3D

#endif //UTIL_H_7708765B_8514_4236_99FE_B8922E68514E