#ifndef MATRIX_H_4A5201AC_394D_4555_9F48_83E2C44C5C87
#define MATRIX_H_4A5201AC_394D_4555_9F48_83E2C44C5C87

#include <memory.h>
#include "vector3.h"
#include "vector4.h"
#include "util.h"

namespace Nomad3D
{
	enum EMatrixType
	{
		eNone,
		eIdentity
	};
	
	class CMatrix4
	{
		friend class CQuaternion;
		typedef float CMatrix4_MX[4][4];
		
    protected:
        float m_[4][4];
        
    public:
		inline void InverseTranslateVec(float * fpVec)
		{
			float* m_fMat = (float*)m_;
			fpVec[0] = fpVec[0] - m_fMat[12];
			fpVec[1] = fpVec[1] - m_fMat[13];
			fpVec[2] = fpVec[2] - m_fMat[14];
		}
		inline void InverseRotateVec(float * fpVec)
		{
			float tmp[3];
			float* m_fMat = (float*)m_;
			tmp[0] = fpVec[0] * m_fMat[0] + fpVec[1] * m_fMat[1] + fpVec[2] * m_fMat[2];
			tmp[1] = fpVec[0] * m_fMat[4] + fpVec[1] * m_fMat[5] + fpVec[2] * m_fMat[6];
			tmp[2] = fpVec[0] * m_fMat[8] + fpVec[1] * m_fMat[9] + fpVec[2] * m_fMat[10];
			
			memcpy(fpVec, tmp, sizeof(float[3]));
		}

        CMatrix4(EMatrixType emtype = eNone);
        CMatrix4(const CMatrix4&);
		CMatrix4(float, float, float, float,
            float, float, float, float,
            float, float, float, float,
            float, float, float, float);
        
        // array access
		
        float* operator [](int);
        const float* operator [](int) const;
        const float* AsArray() const;
        
        // assignment
        
        CMatrix4& Assign(float, float, float, float,
            float, float, float, float,
            float, float, float, float,
            float, float, float, float);
        CMatrix4& operator =(const CMatrix4&);
        
        // operators
        
		float operator () (int r, int c) const;
		float& operator () (int r, int c);

        CMatrix4& operator +=(const CMatrix4&);
        CMatrix4& operator -=(const CMatrix4&);
        CMatrix4& operator *=(const CMatrix4&);
        CMatrix4& operator *=(float);
        CMatrix4& operator /=(float);
		
        CMatrix4 operator +(const CMatrix4&) const;
        CMatrix4 operator -(const CMatrix4&) const;
        CMatrix4 operator -() const;
        CMatrix4 operator *(const CMatrix4&) const;
        CMatrix4 operator *(float) const;
        CMatrix4 operator /(float) const;
        
        friend CMatrix4 operator *(float, const CMatrix4&);
		friend inline CVector3 operator *(const CVector3& v, const CMatrix4& m);
		friend inline CVector4 operator *(const CVector4& v, const CMatrix4& m);
		friend inline void operator *=(CVector3& v, const CMatrix4& m);
		friend inline void operator *=(CVector4& v, const CMatrix4& m);
        
        bool operator ==(const CMatrix4&) const;
        bool operator !=(const CMatrix4&) const;
		
		void Identify();
		float Inverse(/*CMatrix4& mOut, */const CMatrix4& rhs);
        
        //CMatrix4 inverse() const;
        CMatrix4 Transpose() const;
		
        void CopyTo(float [4][4] ) const;


		inline void SetRotation(float fX, float fY, float fZ)
		{
			fX = R2D(fX);
			fY = R2D(fY);
			fZ = R2D(fZ);

			float cx = Cos(fX);
			float sx = Sin(fX);
			float cy = Cos(fY);
			float sy = Sin(fY);
			float cz = Cos(fZ);
			float sz = Sin(fZ);
			
			m_[0][0] = (float)(cy * cz);
			m_[0][1] = (float)(cy * sz);
			m_[0][2] = (float)(-sy);
			
			m_[1][0] = (float)(sx * sy * cz - cx * sz);
			m_[1][1] = (float)(sx * sy * sz + cx * cz);
			m_[1][2] = (float)(sx * cy);
			
			m_[2][0] = (float)(cx * sy * cz + sx * sz);
			m_[2][1] = (float)(cx * sy * sz - sx * cz);
			m_[2][2] = (float)(cx * cy);
			
			m_[3][3] = 1.0f;

		}
        
		//Rotate around arbitrary vector u with angle
		static void Rotate(float angle, const CVector4& u, CMatrix4& rot)
		{
			float c = Cos(angle);
			float s = Sin(angle);

			float ux = u.x();
			float uy = u.y();
			float uz = u.z();

			rot.Assign(
				c+(1-c)*ux*ux,		(1-c)*ux*uy+s*uz,	(1-c)*ux*uz-s*uy,	0,
				(1-c)*uy*ux-s*uz,	c+(1-c)*uy*uy,		(1-c)*uy*uz+s*ux,	0,
				(1-c)*uz*ux+s*uy,	(1-c)*uz*uy-s*ux,	c+(1-c)*uz*uz,		0,
				0,					0,					0,					1
				);
		}

		//Rotate around x , y or z axis
		static void Rotate(float x_angle, float y_angle, float z_angle, CMatrix4& rot)
		{
			CMatrix4 m(eIdentity);
			
			CMatrix4 mx, my, mz, mtmp;       
			float sin_theta=0, cos_theta=0;  
			int nRotateFlag = 0;             
			
			if (!IsZero(x_angle)) // x
				nRotateFlag |= 0x01;
			
			if (!IsZero(y_angle)) // y
				nRotateFlag |= 0x02;
			
			if (!IsZero(z_angle)) // z
				nRotateFlag |= 0x04;
		
			switch(nRotateFlag)
			{
			case 0: // no rotation
				{
					rot = m;
					return;
				} 
				break;
			case 1: // x rotation
				{
					cos_theta = Cos(x_angle);
					sin_theta = Sin(x_angle);
					
					rot.Assign(
						1,    0,          0,         0,
						0,    cos_theta,  sin_theta, 0,
						0,   -sin_theta,  cos_theta, 0,
						0,    0,          0,         1);
					
					return;
				}
				break;
			case 2: // y rotation
				{
					cos_theta = Cos(y_angle);
					sin_theta = Sin(y_angle);
					
					rot.Assign(
						cos_theta, 0, -sin_theta, 0,  
						0,         1, 0,          0,
						sin_theta, 0, cos_theta,  0,
						0,         0, 0,          1);
					
					return;
					
				} 
				break;
			case 3: // xy rotation
				{
					cos_theta = Cos(x_angle);
					sin_theta = Sin(x_angle);
					
					rot.Assign(
						1,    0,          0,         0,
						0,    cos_theta,  sin_theta, 0,
						0,   -sin_theta,  cos_theta, 0,
						0,    0,          0,         1);
					
					
					cos_theta = Cos(y_angle);
					sin_theta = Sin(y_angle);
					
					CMatrix4 roty(
						cos_theta, 0, -sin_theta, 0,  
						0,         1,  0,         0,
						sin_theta, 0, cos_theta,  0,
						0,         0, 0,          1);
					
					// concatenate matrices 
					rot*=roty;
					return;
					
				} 
				break;
			case 4: // z rotation
				{
					cos_theta = Cos(z_angle);
					sin_theta = Sin(z_angle);
					
					rot.Assign(
						cos_theta, sin_theta, 0, 0,  
						-sin_theta, cos_theta,0, 0,
						0,         0,         1, 0,
						0,         0,         0, 1);
					
					return;
					
				} 
				break;
			case 5: // xz rotation
				{
					cos_theta = Cos(x_angle);
					sin_theta = Sin(x_angle);
					
					rot.Assign(
						1,    0,          0,         0,
						0,    cos_theta,  sin_theta, 0,
						0,   -sin_theta,  cos_theta, 0,
						0,    0,          0,         1);
					
					cos_theta = Cos(z_angle);
					sin_theta = Sin(z_angle);
					
					CMatrix4 rotz(
						cos_theta, sin_theta, 0, 0,  
						-sin_theta, cos_theta,0, 0,
						0,         0,         1, 0,
						0,         0,         0, 1);
					
					rot*=rotz;
					return;
				} 
				break;
			case 6: // yz rotation
				{
					cos_theta = Cos(y_angle);
					sin_theta = Sin(y_angle);
					
					rot.Assign(
						cos_theta, 0, -sin_theta, 0,  
						0,         1,  0,         0,
						sin_theta, 0, cos_theta,  0,
						0,         0, 0,          1);
					
					cos_theta = Cos(z_angle);
					sin_theta = Sin(z_angle);
					
					CMatrix4 rotz(
						cos_theta, sin_theta, 0, 0,  
						-sin_theta, cos_theta,0, 0,
						0,         0,         1, 0,
						0,         0,         0, 1);
					
					rot*=rotz;
					return;
					
				} 
				break;
			case 7: // xyz rotation
				{
					cos_theta = Cos(x_angle);
					sin_theta = Sin(x_angle);
					
					rot.Assign(
						1,    0,         0,         0,
						0,    cos_theta, sin_theta, 0,
						0,   -sin_theta, cos_theta, 0,
						0,    0,         0,         1);
					
					cos_theta = Cos(y_angle);
					sin_theta = Sin(y_angle);
					
					CMatrix4 roty(
						cos_theta, 0, -sin_theta, 0,  
						0,         1,  0,         0,
						sin_theta, 0,  cos_theta, 0,
						0,         0,  0,         1);
					
					cos_theta = Cos(z_angle);
					sin_theta = Sin(z_angle);
					
					CMatrix4 rotz(
						cos_theta, sin_theta, 0, 0,  
						-sin_theta, cos_theta,0, 0,
						0,         0,         1, 0,
						0,         0,         0, 1);
					
					rot*=roty;
					rot*=rotz;
					return;			
				}
				break;
			default:
				break;
			}
		}
		static void RotateGL(float x_angle, float y_angle, float z_angle,CMatrix4& rot)
		{
			Rotate(x_angle, y_angle, z_angle, rot);
			rot.Transpose();
		}
        static void Scale(float x_scale, float y_scale, float z_scale, CMatrix4& scal)
		{
			scal.Assign(
				x_scale,	0,		0,		0,
				0,			y_scale, 0,		0,
				0,			0,		z_scale,0,
				0,			0,		0,		1
				);	
		}
		static void ScaleGL(float x_scale, float y_scale, float z_scale, CMatrix4& scal)
		{
			scal.Assign(
				x_scale,	0,		0,		0,
				0,			y_scale, 0,		0,
				0,			0,		z_scale,0,
				0,			0,		0,		1
				);	
		}
        static void Translate(float x_trans, float y_trans, float z_trans, CMatrix4& trans)
		{
			trans.Assign(
				1,			0,		0,		0,
				0,			1,		0,		0,
				0,			0,		1,		0,
				x_trans,	y_trans,z_trans,1
				);
		}
		static void TranslateGL(float x_trans, float y_trans, float z_trans, CMatrix4& trans)
		{
			trans.Assign(
				1,			0,		0,	x_trans,
				0,			1,		0,	y_trans,
				0,			0,		1,	z_trans,
				0,			0,		0,		1
				);
		}
		
	}; // class CMatrix4
	
	inline CMatrix4::CMatrix4(EMatrixType emtype)
	{
		if(emtype == eIdentity)
		{
			memset(m_,0,sizeof(m_));
			m_[0][0]=1;
			m_[1][1]=1;
			m_[2][2]=1;
			m_[3][3]=1;
		}
		else
			memset(m_, 0, sizeof(float[16]));
	};
	
	inline CMatrix4::CMatrix4(const CMatrix4& m)
	{
		memcpy(m_,m.m_,sizeof(m_));
	}
	
	inline CMatrix4::CMatrix4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33)
	{
		m_[0][0]=m00; m_[0][1]=m01; m_[0][2]=m02; m_[0][3]=m03;
		m_[1][0]=m10; m_[1][1]=m11; m_[1][2]=m12; m_[1][3]=m13;
		m_[2][0]=m20; m_[2][1]=m21; m_[2][2]=m22; m_[2][3]=m23;
		m_[3][0]=m30; m_[3][1]=m31; m_[3][2]=m32; m_[3][3]=m33;
	}
	
	inline float* CMatrix4::operator [](int i)
	{
		DEBUG_ASSERT(i == 0 || i == 1 || i == 2 || i == 3 && "Index out of range");
		return &m_[i][0];
	}
	
	inline const float* CMatrix4::operator [](int i) const
	{
		DEBUG_ASSERT(i == 0 || i == 1 || i == 2 || i == 3 && "Index out of range");
		return &m_[i][0];
	}
	
	inline const float* CMatrix4::AsArray() const 
	{ 
		return &m_[0][0];
	}
	
	inline CMatrix4& CMatrix4::Assign(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33)
	{
		m_[0][0]=m00; m_[0][1]=m01; m_[0][2]=m02; m_[0][3]=m03;
		m_[1][0]=m10; m_[1][1]=m11; m_[1][2]=m12; m_[1][3]=m13;
		m_[2][0]=m20; m_[2][1]=m21; m_[2][2]=m22; m_[2][3]=m23;
		m_[3][0]=m30; m_[3][1]=m31; m_[3][2]=m32; m_[3][3]=m33;
		return *this;
	}

	inline float CMatrix4::operator () (int r, int c) const
	{
		return m_[r][c];
	}

	inline float& CMatrix4::operator () (int r, int c)
	{
		return m_[r][c];
	}

	
	inline CMatrix4& CMatrix4::operator =(const CMatrix4& m)
	{
		memcpy(m_,m.m_,sizeof(float)*16);
		return *this;
	}
	
	inline CMatrix4& CMatrix4::operator +=(const CMatrix4& m)
	{
		const CMatrix4_MX& mm=m.m_;
		m_[0][0] += mm[0][0]; m_[0][1] += mm[0][1]; m_[0][2] += mm[0][2]; m_[0][3] += mm[0][3];
		m_[1][0] += mm[1][0]; m_[1][1] += mm[1][1]; m_[1][2] += mm[1][2]; m_[1][3] += mm[1][3];
		m_[2][0] += mm[2][0]; m_[2][1] += mm[2][1]; m_[2][2] += mm[2][2]; m_[2][3] += mm[2][3];
		m_[3][0] += mm[3][0]; m_[3][1] += mm[3][1]; m_[3][2] += mm[3][2]; m_[3][3] += mm[3][3];
		return *this;
	}
	
	inline CMatrix4& CMatrix4::operator -=(const CMatrix4& m)
	{
		const CMatrix4_MX& mm=m.m_;
		m_[0][0] -= mm[0][0]; m_[0][1] -= mm[0][1]; m_[0][2] -= mm[0][2]; m_[0][3] -= mm[0][3];
		m_[1][0] -= mm[1][0]; m_[1][1] -= mm[1][1]; m_[1][2] -= mm[1][2]; m_[1][3] -= mm[1][3];
		m_[2][0] -= mm[2][0]; m_[2][1] -= mm[2][1]; m_[2][2] -= mm[2][2]; m_[2][3] -= mm[2][3];
		m_[3][0] -= mm[3][0]; m_[3][1] -= mm[3][1]; m_[3][2] -= mm[3][2]; m_[3][3] -= mm[3][3];
		return *this;
	}
	
	inline CMatrix4& CMatrix4::operator *=(const CMatrix4& m)
	{
		const CMatrix4_MX& mm = m.m_;
		CMatrix4 rm;
		CMatrix4_MX& rmm = rm.m_;
		
		rmm[0][0] = m_[0][0]*mm[0][0] + m_[0][1]*mm[1][0] + m_[0][2]*mm[2][0] + m_[0][3]*mm[3][0];
		rmm[0][1] = m_[0][0]*mm[0][1] + m_[0][1]*mm[1][1] + m_[0][2]*mm[2][1] + m_[0][3]*mm[3][1];
		rmm[0][2] = m_[0][0]*mm[0][2] + m_[0][1]*mm[1][2] + m_[0][2]*mm[2][2] + m_[0][3]*mm[3][2];
		rmm[0][3] = m_[0][0]*mm[0][3] + m_[0][1]*mm[1][3] + m_[0][2]*mm[2][3] + m_[0][3]*mm[3][3];

		rmm[1][0] = m_[1][0]*mm[0][0] + m_[1][1]*mm[1][0] + m_[1][2]*mm[2][0] + m_[1][3]*mm[3][0];
		rmm[1][1] = m_[1][0]*mm[0][1] + m_[1][1]*mm[1][1] + m_[1][2]*mm[2][1] + m_[1][3]*mm[3][1];
		rmm[1][2] = m_[1][0]*mm[0][2] + m_[1][1]*mm[1][2] + m_[1][2]*mm[2][2] + m_[1][3]*mm[3][2];
		rmm[1][3] = m_[1][0]*mm[0][3] + m_[1][1]*mm[1][3] + m_[1][2]*mm[2][3] + m_[1][3]*mm[3][3];

		rmm[2][0] = m_[2][0]*mm[0][0] + m_[2][1]*mm[1][0] + m_[2][2]*mm[2][0] + m_[2][3]*mm[3][0];
		rmm[2][1] = m_[2][0]*mm[0][1] + m_[2][1]*mm[1][1] + m_[2][2]*mm[2][1] + m_[2][3]*mm[3][1];
		rmm[2][2] = m_[2][0]*mm[0][2] + m_[2][1]*mm[1][2] + m_[2][2]*mm[2][2] + m_[2][3]*mm[3][2];
		rmm[2][3] = m_[2][0]*mm[0][3] + m_[2][1]*mm[1][3] + m_[2][2]*mm[2][3] + m_[2][3]*mm[3][3];

		rmm[3][0] = m_[3][0]*mm[0][0] + m_[3][1]*mm[1][0] + m_[3][2]*mm[2][0] + m_[3][3]*mm[3][0];
		rmm[3][1] = m_[3][0]*mm[0][1] + m_[3][1]*mm[1][1] + m_[3][2]*mm[2][1] + m_[3][3]*mm[3][1];
		rmm[3][2] = m_[3][0]*mm[0][2] + m_[3][1]*mm[1][2] + m_[3][2]*mm[2][2] + m_[3][3]*mm[3][2];
		rmm[3][3] = m_[3][0]*mm[0][3] + m_[3][1]*mm[1][3] + m_[3][2]*mm[2][3] + m_[3][3]*mm[3][3];
		
		//memcpy(m_, rmm, sizeof(m_));
		std::copy(&rmm[0][0], &rmm[0][0] + 16, &m_[0][0]);
		
		return *this;
	}
	
	inline CMatrix4& CMatrix4::operator *=(float f)
	{
		m_[0][0]*=f;
		m_[0][1]*=f;
		m_[0][2]*=f;
		m_[0][3]*=f;
		m_[1][0]*=f;
		m_[1][1]*=f;
		m_[1][2]*=f;
		m_[1][3]*=f;
		m_[2][0]*=f;
		m_[2][1]*=f;
		m_[2][2]*=f;
		m_[2][3]*=f;
		m_[3][0]*=f;
		m_[3][1]*=f;
		m_[3][2]*=f;
		m_[3][3]*=f;
		return *this;
	}
	
	inline CMatrix4& CMatrix4::operator /=(float f)
	{
		m_[0][0]/=f;
		m_[0][1]/=f;
		m_[0][2]/=f;
		m_[0][3]/=f;
		m_[1][0]/=f;
		m_[1][1]/=f;
		m_[1][2]/=f;
		m_[1][3]/=f;
		m_[2][0]/=f;
		m_[2][1]/=f;
		m_[2][2]/=f;
		m_[2][3]/=f;
		m_[3][0]/=f;
		m_[3][1]/=f;
		m_[3][2]/=f;
		m_[3][3]/=f;
		return *this;
	}
	
	inline CMatrix4 CMatrix4::operator +(const CMatrix4& m) const
	{
		CMatrix4 mr;
		const CMatrix4_MX& mm=m.m_;
		mr[0][0]=m_[0][0]+mm[0][0];
		mr[0][1]=m_[0][1]+mm[0][1];
		mr[0][2]=m_[0][2]+mm[0][2];
		mr[0][3]=m_[0][3]+mm[0][3];
		mr[1][0]=m_[1][0]+mm[1][0];
		mr[1][1]=m_[1][1]+mm[1][1];
		mr[1][2]=m_[1][2]+mm[1][2];
		mr[1][3]=m_[1][3]+mm[1][3];
		mr[2][0]=m_[2][0]+mm[2][0];
		mr[2][1]=m_[2][1]+mm[2][1];
		mr[2][2]=m_[2][2]+mm[2][2];
		mr[2][3]=m_[2][3]+mm[2][3];
		mr[3][0]=m_[3][0]+mm[3][0];
		mr[3][1]=m_[3][1]+mm[3][1];
		mr[3][2]=m_[3][2]+mm[3][2];
		mr[3][3]=m_[3][3]+mm[3][3];
		return mr;
	}
	
	inline CMatrix4 CMatrix4::operator -(const CMatrix4& m) const
	{
		CMatrix4 mr;
		const CMatrix4_MX& mm=m.m_;
		mr[0][0]=m_[0][0]-mm[0][0];
		mr[0][1]=m_[0][1]-mm[0][1];
		mr[0][2]=m_[0][2]-mm[0][2];
		mr[0][3]=m_[0][3]-mm[0][3];
		mr[1][0]=m_[1][0]-mm[1][0];
		mr[1][1]=m_[1][1]-mm[1][1];
		mr[1][2]=m_[1][2]-mm[1][2];
		mr[1][3]=m_[1][3]-mm[1][3];
		mr[2][0]=m_[2][0]-mm[2][0];
		mr[2][1]=m_[2][1]-mm[2][1];
		mr[2][2]=m_[2][2]-mm[2][2];
		mr[2][3]=m_[2][3]-mm[2][3];
		mr[3][0]=m_[3][0]-mm[3][0];
		mr[3][1]=m_[3][1]-mm[3][1];
		mr[3][2]=m_[3][2]-mm[3][2];
		mr[3][3]=m_[3][3]-mm[3][3];
		return mr;
	}
	
	inline CMatrix4 CMatrix4::operator -() const
	{
		CMatrix4 mm;
		CMatrix4_MX& m=mm.m_;
		m[0][0]=-m_[0][0];
		m[0][1]=-m_[0][1];
		m[0][2]=-m_[0][2];
		m[0][3]=-m_[0][3];
		m[1][0]=-m_[1][0];
		m[1][1]=-m_[1][1];
		m[1][2]=-m_[1][2];
		m[1][3]=-m_[1][3];
		m[2][0]=-m_[2][0];
		m[2][1]=-m_[2][1];
		m[2][2]=-m_[2][2];
		m[2][3]=-m_[2][3];
		m[3][0]=-m_[3][0];
		m[3][1]=-m_[3][1];
		m[3][2]=-m_[3][2];
		m[3][3]=-m_[3][3];
		return mm;
	}
	
	inline CMatrix4 CMatrix4::operator *(const CMatrix4& m) const
	{
		CMatrix4 mr;
		
		CMatrix4_MX& rmm = mr.m_;
		const CMatrix4_MX& mm = m.m_;
		
		rmm[0][0] = m_[0][0]*mm[0][0] + m_[0][1]*mm[1][0] + m_[0][2]*mm[2][0] + m_[0][3]*mm[3][0];
		rmm[0][1] = m_[0][0]*mm[0][1] + m_[0][1]*mm[1][1] + m_[0][2]*mm[2][1] + m_[0][3]*mm[3][1];
		rmm[0][2] = m_[0][0]*mm[0][2] + m_[0][1]*mm[1][2] + m_[0][2]*mm[2][2] + m_[0][3]*mm[3][2];
		rmm[0][3] = m_[0][0]*mm[0][3] + m_[0][1]*mm[1][3] + m_[0][2]*mm[2][3] + m_[0][3]*mm[3][3];

		rmm[1][0] = m_[1][0]*mm[0][0] + m_[1][1]*mm[1][0] + m_[1][2]*mm[2][0] + m_[1][3]*mm[3][0];
		rmm[1][1] = m_[1][0]*mm[0][1] + m_[1][1]*mm[1][1] + m_[1][2]*mm[2][1] + m_[1][3]*mm[3][1];
		rmm[1][2] = m_[1][0]*mm[0][2] + m_[1][1]*mm[1][2] + m_[1][2]*mm[2][2] + m_[1][3]*mm[3][2];
		rmm[1][3] = m_[1][0]*mm[0][3] + m_[1][1]*mm[1][3] + m_[1][2]*mm[2][3] + m_[1][3]*mm[3][3];

		rmm[2][0] = m_[2][0]*mm[0][0] + m_[2][1]*mm[1][0] + m_[2][2]*mm[2][0] + m_[2][3]*mm[3][0];
		rmm[2][1] = m_[2][0]*mm[0][1] + m_[2][1]*mm[1][1] + m_[2][2]*mm[2][1] + m_[2][3]*mm[3][1];
		rmm[2][2] = m_[2][0]*mm[0][2] + m_[2][1]*mm[1][2] + m_[2][2]*mm[2][2] + m_[2][3]*mm[3][2];
		rmm[2][3] = m_[2][0]*mm[0][3] + m_[2][1]*mm[1][3] + m_[2][2]*mm[2][3] + m_[2][3]*mm[3][3];

		rmm[3][0] = m_[3][0]*mm[0][0] + m_[3][1]*mm[1][0] + m_[3][2]*mm[2][0] + m_[3][3]*mm[3][0];
		rmm[3][1] = m_[3][0]*mm[0][1] + m_[3][1]*mm[1][1] + m_[3][2]*mm[2][1] + m_[3][3]*mm[3][1];
		rmm[3][2] = m_[3][0]*mm[0][2] + m_[3][1]*mm[1][2] + m_[3][2]*mm[2][2] + m_[3][3]*mm[3][2];
		rmm[3][3] = m_[3][0]*mm[0][3] + m_[3][1]*mm[1][3] + m_[3][2]*mm[2][3] + m_[3][3]*mm[3][3];
		
		memcpy(mr.m_, rmm, sizeof(mr.m_));
		return mr;
	}
	
	inline CMatrix4 CMatrix4::operator *(float f) const
	{
		CMatrix4 mr(*this);
		CMatrix4_MX& mm=mr.m_;
		mm[0][0]*=f;
		mm[0][1]*=f;
		mm[0][2]*=f;
		mm[0][3]*=f;
		mm[1][0]*=f;
		mm[1][1]*=f;
		mm[1][2]*=f;
		mm[1][3]*=f;
		mm[2][0]*=f;
		mm[2][1]*=f;
		mm[2][2]*=f;
		mm[2][3]*=f;
		mm[3][0]*=f;
		mm[3][1]*=f;
		mm[3][2]*=f;
		mm[3][3]*=f;
		return mr;
	}
	
	inline CMatrix4 CMatrix4::operator /(float f) const
	{
		CMatrix4 mr(*this);
		CMatrix4_MX& mm=mr.m_;
		mm[0][0]*=f;
		mm[0][1]*=f;
		mm[0][2]*=f;
		mm[0][3]*=f;
		mm[1][0]*=f;
		mm[1][1]*=f;
		mm[1][2]*=f;
		mm[1][3]*=f;
		mm[2][0]*=f;
		mm[2][1]*=f;
		mm[2][2]*=f;
		mm[2][3]*=f;
		mm[3][0]*=f;
		mm[3][1]*=f;
		mm[3][2]*=f;
		mm[3][3]*=f;
		return mr;
	}
	
	//this is a friend function of CMatrix4
	inline CMatrix4 operator *(float f, const CMatrix4& m)
	{
		return m*f;
	}
	//this is a friend function of CMatrix4
	
	inline CVector3 operator *(const CVector3& v, const CMatrix4& m)
	{/*
		const CMatrix4::CMatrix4_MX& mm = m.m_;
		float w = 1.0f / (v[0]*mm[0][3] + v[1]*mm[1][3] + v[2]*mm[2][3] + mm[3][3]);
		CVector3 r(
			(v[0]*mm[0][0] + v[1]*mm[1][0] + v[2]*mm[2][0] + mm[3][0]) * w,
			(v[0]*mm[0][1] + v[1]*mm[1][1] + v[2]*mm[2][1] + mm[3][1]) * w,
			(v[0]*mm[0][2] + v[1]*mm[1][2] + v[2]*mm[2][2] + mm[3][2]) * w
			);
		return r;*/
		const CMatrix4::CMatrix4_MX& mm = m.m_;
		CVector3 r(
			(v[0]*mm[0][0] + v[1]*mm[1][0] + v[2]*mm[2][0]),
			(v[0]*mm[0][1] + v[1]*mm[1][1] + v[2]*mm[2][1]),
			(v[0]*mm[0][2] + v[1]*mm[1][2] + v[2]*mm[2][2])
			);
		return r;
	}

	inline CVector4 operator *(const CVector4& v, const CMatrix4& m)
	{
		const CMatrix4::CMatrix4_MX& mm = m.m_;
		CVector4 r(
			v[0]*mm[0][0] + v[1]*mm[1][0] + v[2]*mm[2][0] + v[3]*mm[3][0],
			v[0]*mm[0][1] + v[1]*mm[1][1] + v[2]*mm[2][1] + v[3]*mm[3][1],
			v[0]*mm[0][2] + v[1]*mm[1][2] + v[2]*mm[2][2] + v[3]*mm[3][2],
			v[0]*mm[0][3] + v[1]*mm[1][3] + v[2]*mm[2][3] + v[3]*mm[3][3]
			);
		return r;
	}

	////////////////////////////////////////////////////
	inline void operator *=(CVector3& v, const CMatrix4& m)
	{/*
		const CMatrix4::CMatrix4_MX& mm = m.m_;
		float w = 1.0f / (v[0]*mm[0][3] + v[1]*mm[1][3] + v[2]*mm[2][3] + mm[3][3]);
		CVector3 r(
			(v[0]*mm[0][0] + v[1]*mm[1][0] + v[2]*mm[2][0] + mm[3][0]) * w,
			(v[0]*mm[0][1] + v[1]*mm[1][1] + v[2]*mm[2][1] + mm[3][1]) * w,
			(v[0]*mm[0][2] + v[1]*mm[1][2] + v[2]*mm[2][2] + mm[3][2]) * w
			);
		
		v = r;*/
		const CMatrix4::CMatrix4_MX& mm = m.m_;
		CVector3 r(
			(v[0]*mm[0][0] + v[1]*mm[1][0] + v[2]*mm[2][0]),
			(v[0]*mm[0][1] + v[1]*mm[1][1] + v[2]*mm[2][1]),
			(v[0]*mm[0][2] + v[1]*mm[1][2] + v[2]*mm[2][2])
			);
		
		v = r;
	}//////////////////////////////////////////////////////

	inline void operator *=(CVector4& v, const CMatrix4& m)
	{
		const CMatrix4::CMatrix4_MX& mm = m.m_;
		CVector4 r(
			v[0]*mm[0][0] + v[1]*mm[1][0] + v[2]*mm[2][0] + v[3]*mm[3][0],
			v[0]*mm[0][1] + v[1]*mm[1][1] + v[2]*mm[2][1] + v[3]*mm[3][1],
			v[0]*mm[0][2] + v[1]*mm[1][2] + v[2]*mm[2][2] + v[3]*mm[3][2],
			v[0]*mm[0][3] + v[1]*mm[1][3] + v[2]*mm[2][3] + v[3]*mm[3][3]
			);
		v = r;
	}
	
	inline bool CMatrix4::operator ==(const CMatrix4& m) const
	{
		int i,j;
		const CMatrix4_MX& mm = m.m_;
		for(i=0; i<4; i++)
		{
			for(j=0; j<4; j++)
			{
				if(!FuzEQ(m_[i][j],mm[i][j]))
					return false;
			}
		}
		return true;
	}
	
	inline bool CMatrix4::operator !=(const CMatrix4& m) const
	{
		return !((*this)==m);
	}
	
	inline void CMatrix4::Identify()
	{
		memset(m_,0,sizeof(m_));
		m_[0][0]=1;
		m_[1][1]=1;
		m_[2][2]=1;
		m_[3][3]=1;
	}
	
	inline CMatrix4 CMatrix4::Transpose() const
	{
		CMatrix4 tmp(
			m_[0][0],m_[1][0],m_[2][0],m_[3][0],
			m_[0][1],m_[1][1],m_[2][1],m_[3][1],
			m_[0][2],m_[1][2],m_[2][2],m_[3][2],
			m_[0][3],m_[1][3],m_[2][3],m_[3][3]);
		
		return tmp;
	}
	
	inline void CMatrix4::CopyTo(float m[4][4] ) const
	{
		memcpy(m,m_,sizeof(m_));
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	inline float CMatrix4::Inverse(/*CMatrix4& mOut, */const CMatrix4& rhs)
	{ 
		CMatrix4 m(rhs); 
		int is[4]; 
		int js[4]; 
		float fDet = 1.0f; 
		int f = 1; 
		int i,j,k;
		for (k = 0; k < 4; k ++) 
		{ 
			// ???,???? 
			float fMax = 0.0f; 
			for (i = k; i < 4; i ++) 
			{ 
				for (j = k; j < 4; j ++) 
				{
					const float f = Abs(m(i, j));
					if (f > fMax)
					{ fMax = f;
					is[k] = i;
					js[k] = j;
					}
					
				}
			}
			if (Abs(fMax) < 0.0001f)
				return 0;
			if (is[k] != k)
			{
				f = -f;
				SWAP(m(k, 0), m(is[k], 0));
				SWAP(m(k, 1), m(is[k], 1));
				SWAP(m(k, 2), m(is[k], 2));
				SWAP(m(k, 3), m(is[k], 3));
			}
			if (js[k] != k)
			{
				f = -f;
				SWAP(m(0, k), m(0, js[k]));
				SWAP(m(1, k), m(1, js[k]));
				SWAP(m(2, k), m(2, js[k]));
				SWAP(m(3, k), m(3, js[k]));
			}
			// ?????
			fDet *= m(k, k);
			// ?????
			// ???
			m(k, k) = 1.0f / m(k, k); 
			// ???
			for (j = 0; j < 4; j ++)
			{
				if (j != k)
					m(k, j) *= m(k, k);
			}
			// ???
			for (i = 0; i < 4; i ++)
			{
				if (i != k)
				{
					for (j = 0; j < 4; j ++)
					{
						if (j != k)
							m(i, j) = m(i, j) - m(i, k) * m(k, j); }
				}
			}
			// ???
			for (i = 0; i < 4; i ++)
			{
				if (i != k)
					m(i, k) *= -m(k, k);
			}
		}
		for (k = 3; k >= 0; k --)
		{
			if (js[k] != k)
			{
				SWAP(m(k, 0), m(js[k], 0));
				SWAP(m(k, 1), m(js[k], 1));
				SWAP(m(k, 2), m(js[k], 2));
				SWAP(m(k, 3), m(js[k], 3));
			}
			if (is[k] != k)
			{ SWAP(m(0, k), m(0, is[k]));
			SWAP(m(1, k), m(1, is[k]));
			SWAP(m(2, k), m(2, is[k]));
			SWAP(m(3, k), m(3, is[k]));
			}
		}
		*this = m;
		return fDet * f;
	}

	
}//namespace Nomad3D

#endif //MATRIX_H_4A5201AC_394D_4555_9F48_83E2C44C5C87