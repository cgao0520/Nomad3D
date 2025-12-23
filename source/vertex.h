// vertex.h: interface for the CVertex3 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VERTEX_H_E365E472_C713_4A19_82F9_5417EC37E3A0)
#define VERTEX_H_E365E472_C713_4A19_82F9_5417EC37E3A0


#include "vector3.h"
#include "vector4.h"
#include "matrix4.h"

namespace Nomad3D
{
#pragma pack(push, packing)
#pragma pack(1)

	class CPolygon;
	class CLight;
	class CRenderList;
	class CVertex4 : public CVector4  
	{
		friend class CPolygon;
		friend class CLight;
		friend class CRenderList;
		friend inline CVertex4 operator *(const CVertex4& v, const CMatrix4& m);
		friend inline void operator *=(CVertex4& v, const CMatrix4& m);
	public:
		CVertex4(){};
		CVertex4(const CVector4& v):CVector4(v){m_cBone = NM3D_JOINT_NO_PARENT;}
		CVertex4(float* pv, char cBone):CVector4(pv),m_cBone(cBone){};
		inline void Assign(float x, float y, float z, float w, char cBone)
		{
			(void)w;
			CVector4::Assign(x,y,z,1);
			m_cBone = cBone;
		}
		inline void Assign(const float* pv, char cBone)
		{
			Assign(pv[0],pv[1],pv[2],1,cBone);
		}

		inline char GetBone()
		{
			return m_cBone;
		}
		inline void SetBone(char cBone)
		{
			m_cBone = cBone;
		}
		inline const CVector3& GetNormal()
		{
			return m_vNormal;
		}
		inline void SetNormal(const CVector3& vNormal)
		{
			m_vNormal = vNormal;
		}

	protected:
		char		m_cBone;
		CVector3	m_vNormal;//normal vector
	};

#pragma pack(pop, packing)

	//////////////////////////////////////////////////////////////////////////
	// CVertex4 * CMatrix4
	inline CVertex4 operator *(const CVertex4& v, const CMatrix4& m)
	{
		CVertex4 rv = v;
		memcpy(rv.v_, (operator *(*((CVector4*)(&rv)), m)).Get(), sizeof(float)*4);
		rv.m_vNormal *= m;
		return rv;
	}
	inline void operator *=(CVertex4& v, const CMatrix4& m)
	{
		memcpy(v.v_, (operator *(*((CVector4*)(&v)), m)).Get(), sizeof(float)*4);
		v.m_vNormal *= m;
	}
}

#endif // !defined(VERTEX_H_E365E472_C713_4A19_82F9_5417EC37E3A0)
