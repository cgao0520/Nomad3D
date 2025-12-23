#ifndef VECTOR3_H_2E436DA1_74EA_4b85_8835_6459CFA736B1
#define VECTOR3_H_2E436DA1_74EA_4b85_8835_6459CFA736B1

#define DEBUG_ASSERT(x)

#include "util.h"

namespace Nomad3D
{
	class CVector4;

	class CVector3 
	{
		friend class CVector4;
	public:
		inline CVector3()
		{
			v_[0] = v_[1] = v_[2] = 0;
		}
		~CVector3(){}

		inline CVector3(float x, float y, float z)
		{
			v_[0] = x; v_[1] = y; v_[2] = z;
		}

		inline CVector3(const float* pv)
		{
			v_[0] = pv[0]; v_[1] = pv[1]; v_[2] = pv[2];
		}
		
		// Component Access:
		
		inline float x() const {return v_[0];}
		
		inline float y() const {return v_[1];}
		
		inline float z() const {return v_[2];}
		
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
		
		inline CVector3& Assign(float x, float y, float z)
		{
			v_[0] = x; v_[1] = y; v_[2] = z;
			return *this;
		}

		inline void Assign(const float* pv)
		{
			v_[0] = pv[0]; v_[1] = pv[1]; v_[2] = pv[2];
		}

		// MATH OPERATORS
		
		inline CVector3& operator +=(const CVector3& v)
		{
			v_[0] += v[0]; v_[1] += v[1]; v_[2] += v[2];
			return *this;
		}
		
		inline CVector3& operator -=(const CVector3& v)
		{
			v_[0] -= v[0]; v_[1] -= v[1]; v_[2] -= v[2];
			return *this;
		}
		
		inline CVector3& operator *=(float c)
		{
			v_[0] *= c; v_[1] *= c; v_[2] *= c;
			return *this;
		}
		
		inline CVector3& operator /=(float c)
		{
			v_[0] /= c; v_[1] /= c; v_[2] /= c;
			return *this;
		}
		
		inline CVector3 operator +(const CVector3& v) const
		{
			return CVector3(v_[0] + v[0], v_[1] + v[1], v_[2] + v[2]);
		}
		
		inline CVector3 operator -(const CVector3& v) const
		{
			return CVector3(v_[0] - v.v_[0], v_[1] - v.v_[1], v_[2] - v.v_[2]);
		}
		
		inline CVector3 operator -() const
		{
			return CVector3(-v_[0], -v_[1], -v_[2]);
		}
		
		inline CVector3 operator *(float c) const
		{
			return CVector3(v_[0] * c, v_[1] * c, v_[2] * c);
		}

		inline float operator *(const CVector3& v) const
		{
			return v_[0]*v.v_[0] + v_[1]*v.v_[1] + v_[2]*v.v_[2];
		}
		
		inline CVector3 operator /(float c) const
		{
			return CVector3(v_[0] / c, v_[1] / c, v_[2] / c);
		}
		
		inline bool operator ==(const CVector3& v) const
		{
			return (FuzEQ(v_[0], v[0]) && FuzEQ(v_[1], v[1]) && FuzEQ(v_[2], v[2]));
		}
		
		inline bool operator !=(const CVector3& v) const
		{
			return (!(*this == v));
		}
		
		// OPERATIONS

		inline float Length() const
		{
			return Sqrt(Sqr(v_[0]) + Sqr(v_[1]) + Sqr(v_[2]));
		}
		
		inline float LengthSquared() const
		{
			return Sqr(v_[0]) + Sqr(v_[1]) + Sqr(v_[2]);
		}
		
		inline CVector3& Normalize()
		{
			*this /= Length();
			return *this;
		}
		
		inline void LowPrecisionNormalise()
		{
			Normalize();
		}
		
		void CopyTo(float f[3]) const
		{
			f[0] = v_[0]; f[1] = v_[1]; f[2] = v_[2];
		}
		
		//friend function

		friend CVector3 Cross(const CVector3&, const CVector3&);
		friend void CrossTo(const CVector3& v1, const CVector3& v2, CVector3& v3);
		friend void NormalTo(const CVector3& v1, const CVector3& vbase, const CVector3& v2, CVector3& v3);
		friend void NonUnitNormalTo(const CVector3& v1, const CVector3& vbase, const CVector3& v2, CVector3& v3);
		friend float Distance(const CVector3&, const CVector3&);
		friend float DistanceSquared(const CVector3&, const CVector3&);
		friend float Dot(const CVector3&, const CVector3&);
		friend CVector3 Lerp(float, const CVector3&, const CVector3&);

	protected:		
		float v_[3];
	};
	
	inline CVector3 Cross(const CVector3& v1, const CVector3& v2)
	{
		return CVector3(
			v1[1] * v2[2] - v1[2] * v2[1],
			v1[2] * v2[0] - v1[0] * v2[2],
			v1[0] * v2[1] - v1[1] * v2[0]);
	}
	
	inline void CrossTo(const CVector3& v1, const CVector3& v2, CVector3& v3)
	{
		
		v3[0] = v1[1] * v2[2] - v1[2] * v2[1];
		
		v3[1] = v1[2] * v2[0] - v1[0] * v2[2];
		
		v3[2] = v1[0] * v2[1] - v1[1] * v2[0];
		
	}
	
	inline void NormalTo(const CVector3& v1, const CVector3& vbase, const CVector3& v2, CVector3& v3)
	{
		CVector3 to_v1 = v1 - vbase;
		
		CVector3 to_v2 = v2 - vbase;
		
		v3 = Cross(to_v1, to_v2);
		
		v3.Normalize();
	}
	
	inline void NonUnitNormalTo(const CVector3& v1, const CVector3& vbase, const CVector3& v2, CVector3& v3)
	{
		CVector3 to_v1 = v1 - vbase;
		
		CVector3 to_v2 = v2 - vbase;
		
		v3 = Cross(to_v1, to_v2);
	}
	
	//equal to two points' distance
	inline float Distance(const CVector3& v1, const CVector3& v2)
	{
		return Sqrt(Sqr(v1[0] - v2[0]) + Sqr(v1[1] - v2[1]) + Sqr(v1[2] - v2[2]));
	}
	
	inline float DistanceSquared(const CVector3& v1, const CVector3& v2)
	{
		return Sqr(v1[0] - v2[0]) + Sqr(v1[1] - v2[1]) + Sqr(v1[2] - v2[2]);
	}
	
	inline float Dot(const CVector3& v1, const CVector3& v2)
	{
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}
	
	inline CVector3 Lerp(float f, const CVector3& v1, const CVector3& v2)
	{
		return v1 + ((v2 - v1) * f);
	}

	typedef CVector3	CPoint3;

}// namespace Nomad3D


#endif //VECTOR3_H_2E436DA1_74EA_4b85_8835_6459CFA736B1
