// vector4.h: interface for the CVector4 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VECTOR4_H_A01F0E90_8040_4ACC_8A81_15FF34D69AC1)
#define VECTOR4_H_A01F0E90_8040_4ACC_8A81_15FF34D69AC1

#include "vector3.h"

namespace Nomad3D
{
	class CVector4 //: public CVector3
	{
	public:
		inline CVector4()
		{
			v_[0] = v_[1] = v_[2] = 0;
			v_[3] = 1; 
		}

		inline CVector4(const CVector3& v)
		{
			v_[0] = v.v_[0]; v_[1] = v.v_[1]; v_[2] = v.v_[2]; 
			v_[3] = 1;
		}

		inline CVector4(const CVector4& v)
		{
			v_[0] = v.v_[0]; v_[1] = v.v_[1]; v_[2] = v.v_[2];
			v_[3] = v.v_[3];
		}
		
		inline CVector4(float x, float y, float z)
		{
			v_[0] = x; v_[1] = y; v_[2] = z;
			v_[3] = 1;
		}

		inline CVector4(float x, float y, float z, float w)
		{
			v_[0] = x; v_[1] = y; v_[2] = z;
			v_[3] = w;
		}

		inline CVector4(const float* pv)
		{
			v_[0] = pv[0]; v_[1] = pv[1]; v_[2] = pv[2];
			v_[3] = pv[3];
		}
		
		//////////////////////////////////////////////////////////////////////////
		// Componant Access:
		
		inline float x() const {return v_[0];}
		
		inline float y() const {return v_[1];}
		
		inline float z() const {return v_[2];}
		
		inline float w() const {return v_[3];}
		
		inline float* Get() { return v_;}
		
		inline float& operator [](int i) 
		{
			return v_[i];
		}
		
		inline const float& operator [](int i) const
		{
			return v_[i];
		}
		
		// ASSIGNMENT
		
		inline CVector4& Assign(float x, float y, float z, float w)
		{
			v_[0] = x; v_[1] = y; v_[2] = z;
			v_[3] = w;
			return *this;
		}

		inline void Assign(const float* pv)
		{
			v_[0] = pv[0]; v_[1] = pv[1]; v_[2] = pv[2];
			v_[3] = 1;
		}
		
		inline CVector4& operator =(const CVector4& v)
		{
			v_[0] = v[0]; v_[1] = v[1]; v_[2] = v[2];
			v_[3] = v.v_[3];
			return *this;
		}
		
		// MATH OPERATORS
		
		inline CVector4& operator +=(const CVector4& v)
		{
			v_[0] += v[0]; v_[1] += v[1]; v_[2] += v[2];
			return *this;
		}
		
		inline CVector4& operator -=(const CVector4& v)
		{
			v_[0] -= v[0]; v_[1] -= v[1]; v_[2] -= v[2];
			return *this;
		}
		
		inline CVector4& operator *=(float c)
		{
			v_[0] *= c; v_[1] *= c; v_[2] *= c;
			return *this;
		}
		
		inline CVector4& operator /=(float c)
		{
			v_[0] /= c; v_[1] /= c; v_[2] /= c;
			return *this;
		}
		
		inline CVector4 operator +(const CVector4& v) const
		{
			return CVector4(v_[0] + v[0], v_[1] + v[1], v_[2] + v[2]);
		}
		
		inline CVector4 operator -(const CVector4& v) const
		{
			return CVector4(v_[0] - v.v_[0], v_[1] - v.v_[1], v_[2] - v.v_[2]);
		}
		
		inline CVector4 operator -() const
		{
			return CVector4(-v_[0], -v_[1], -v_[2]);
		}
		
		inline CVector4 operator *(float c) const
		{
			return CVector4(v_[0] * c, v_[1] * c, v_[2] * c);
		}

		inline float operator *(const CVector4& v) const
		{
			return v_[0]*v.v_[0] + v_[1]*v.v_[1] + v_[2]*v.v_[2];
		}
		
		inline CVector4 operator /(float c) const
		{
			return CVector4(v_[0] / c, v_[1] / c, v_[2] / c);
		}
		
		inline bool operator ==(const CVector4& v) const
		{
			return (FuzEQ(v_[0]/v_[3], v[0]/v[3]) && \
				FuzEQ(v_[1]/v_[3], v[1]/v[3]) && \
				FuzEQ(v_[2]/v_[3], v[2]/v[3]));
		}
		
		inline bool operator !=(const CVector4& v) const
		{
			return (!(*this == v));
		}
		
		// OPERATIONS

		inline float Length() const
		{
			return Sqrt(LengthSquared());
		}
		
		inline float LengthSquared() const
		{
			return Sqr(v_[0]) + Sqr(v_[1]) + Sqr(v_[2]);
		}
		
		inline CVector4& Normalize()
		{
			*this /= Length();
			return *this;
		}
		
		inline void LowPrecisionNormalise()
		{
			Normalize();
		}

		inline void DivideW()
		{
			float w1 =1.0f/v_[3];
			v_[0] *= w1;
			v_[1] *= w1;
			v_[2] *= w1;
			v_[3] = 1.0f;
		}
		
		void CopyTo(float f[4]) const
		{
			f[0] = v_[0]; f[1] = v_[1]; f[2] = v_[2]; f[3] = v_[3];
		}
		
		//friend function

		friend CVector4 Cross4(const CVector4&, const CVector4&);
		friend void CrossTo4(const CVector4& v1, const CVector4& v2, CVector4& v3);
		friend void NormalTo4(const CVector4& v1, const CVector4& vbase, const CVector4& v2, CVector4& v3);
		friend void NonUnitNormalTo4(const CVector4& v1, const CVector4& vbase, const CVector4& v2, CVector4& v3);
		friend float Distance4(const CVector4&, const CVector4&);
		friend float DistanceSquared4(const CVector4&, const CVector4&);
		friend float Dot4(const CVector4&, const CVector4&);
		friend CVector4 Lerp4(float, const CVector4&, const CVector4&);
		friend CVector4 operator - (const CVector3& v1, const CVector4& v2);
		friend CVector4 operator - (const CVector4& v1, const CVector3& v2);

	protected:		
		float v_[4];
	};
	
	inline CVector4 Cross4(const CVector4& v1, const CVector4& v2)
	{
		return CVector4(
			v1[1] * v2[2] - v1[2] * v2[1],
			v1[2] * v2[0] - v1[0] * v2[2],
			v1[0] * v2[1] - v1[1] * v2[0]);
	}
	
	inline void CrossTo4(const CVector4& v1, const CVector4& v2, CVector4& v3)
	{
		
		v3[0] = v1[1] * v2[2] - v1[2] * v2[1];
		
		v3[1] = v1[2] * v2[0] - v1[0] * v2[2];
		
		v3[2] = v1[0] * v2[1] - v1[1] * v2[0];
		
	}
	
	inline void NormalTo4(const CVector4& v1, const CVector4& vbase, const CVector4& v2, CVector4& v3)
	{
		CVector4 to_v1 = v1 - vbase;
		
		CVector4 to_v2 = v2 - vbase;
		
		v3 = Cross4(to_v1, to_v2);
		
		v3.Normalize();
	}
	
	inline void NonUnitNormalTo4(const CVector4& v1, const CVector4& vbase, const CVector4& v2, CVector4& v3)
	{
		CVector4 to_v1 = v1 - vbase;
		
		CVector4 to_v2 = v2 - vbase;
		
		v3 = Cross4(to_v1, to_v2);
	}
	
	//equal to two points' distance
	inline float Distance4(const CVector4& v1, const CVector4& v2)
	{
		return Sqrt(Sqr(v1[0] - v2[0]) + Sqr(v1[1] - v2[1]) + Sqr(v1[2] - v2[2]));
	}
	
	inline float DistanceSquared4(const CVector4& v1, const CVector4& v2)
	{
		return Sqr(v1[0] - v2[0]) + Sqr(v1[1] - v2[1]) + Sqr(v1[2] - v2[2]);
	}
	
	inline float Dot4(const CVector4& v1, const CVector4& v2)
	{
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}
	
	inline CVector4 Lerp4(float f, const CVector4& v1, const CVector4& v2)
	{
		return v1 + ((v2 - v1) * f);
	}

	inline CVector4 operator - (const CVector3& v1, const CVector4& v2)
	{
		return CVector4(v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2]);
	}

	inline CVector4 operator - (const CVector4& v1, const CVector3& v2)
	{
		return CVector4(v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2]);
	}

	typedef CVector4	CPoint4;
};


#endif // !defined(VECTOR4_H_A01F0E90_8040_4ACC_8A81_15FF34D69AC1)
