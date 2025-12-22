
#include "util.h"
#include "constant.h"

namespace Nomad3D
{
	unsigned char Log2(int x)
	{
		return LUT_LogBase2[x];
	}

	float Sin(float angle)// range from 0 to 360
	{
		//return (float)sinf(angle*NM3D_PI/180.0);
		//return LUT_Sin[((int)angle)%360];
		
		angle = fmodf(angle,360);
		if (angle < 0) angle+=360.0;
		int theta_int    = (int)angle;
		float theta_frac = angle - theta_int;
		
		return(LUT_Sin[theta_int] + 
			theta_frac*(LUT_Sin[theta_int+1] - LUT_Sin[theta_int]));
	}
	
	float Cos(float angle)// range from 0 to 360
	{
		//return (float)cosf(angle*NM3D_PI/180.0);
		//return LUT_Cos[((int)angle)%360];
		
		angle = fmodf(angle,360);
		if (angle < 0) angle+=360.0;
		int theta_int    = (int)angle;
		float theta_frac = angle - theta_int;
		
		return(LUT_Cos[theta_int] + 
			theta_frac*(LUT_Cos[theta_int+1] - LUT_Cos[theta_int]));
	}
	
	float Tan(float angle)
	{
		return Sin(angle)/Cos(angle);
	}
	
	float SinR(float angle)
	{
		return Sin(R2D(angle));
	}
	float CosR(float angle)
	{
		return Cos(R2D(angle));
	}
}