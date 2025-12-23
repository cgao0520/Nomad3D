// polygon.h: interface for the CPolygon class.
//
//////////////////////////////////////////////////////////////////////

#ifndef POLYGON_H_4A795054_795A_4E45_B9E3_9C67C41C18BC
#define POLYGON_H_4A795054_795A_4E45_B9E3_9C67C41C18BC

#include "vector3.h"
#include "rgb.h"
#include "material.h"
#include "vertex.h"

namespace Nomad3D
{
#pragma pack(push, packing)
#pragma pack(1)

#define NM3D_NORMAL_POLYGON		0x00000000
#define NM3D_BACK_POLYGON		0x00000001
#define NM3D_CULLED_POLYGON		0x80000000

	class CLight;
	class CRenderList;

	class CPolygon
	{
		friend class CLight;
		friend class CRenderList;
		inline void InitData()
		{
			m_nState = NM3D_NORMAL_POLYGON;
			m_pMaterial = NULL;
			m_pVertList = NULL;

// 			for(int i=0; i<3; i++)
// 				m_rgbaSpecular[i] = CRGBA(0,0,0,1);
		}
	public:
		CPolygon()
		{
			InitData();
		}

		CPolygon(const unsigned short* pusVIndices, const float* pVNormals, const float* pTCoords)
		{
			(void)pVNormals;
			memcpy(m_usVertIndices,pusVIndices,sizeof(m_usVertIndices));
//			memcpy(m_vNormals,pVNormals,sizeof(m_vNormals));
			memcpy(m_fTexCoords,pTCoords,sizeof(m_fTexCoords));

			InitData();
		}

		void Assign(const unsigned short* pusVIndices, const float* pVNormals, const float* pTCoords)
		{
			(void)pVNormals;
			memcpy(m_usVertIndices,pusVIndices,sizeof(m_usVertIndices));
//			memcpy(m_vNormals,pVNormals,sizeof(m_vNormals));
			memcpy(m_fTexCoords,pTCoords,sizeof(m_fTexCoords));

			InitData();
		}

		void AssignWithInitVertexNormal(CVertex4* pVertList, const unsigned short* pusVIndices, const float* pVNormals, const float* pTCoords)
		{
			memcpy(m_usVertIndices, pusVIndices, sizeof(m_usVertIndices));

			CVector3 vNormal0(pVNormals);
			CVector3 vNormal1(pVNormals + 3);
			CVector3 vNormal2(pVNormals + 6);
			
			memcpy(m_fTexCoords, pTCoords, sizeof(m_fTexCoords));

			pVertList[m_usVertIndices[0]].m_vNormal = vNormal0;
			pVertList[m_usVertIndices[1]].m_vNormal = vNormal1;
			pVertList[m_usVertIndices[2]].m_vNormal = vNormal2;

			InitData();
		}

		inline const unsigned short* GetVertIndices() const
		{
			return m_usVertIndices;
		}

		inline void AddState(unsigned int state)
		{
			m_nState |= state;
		}

		inline unsigned int GetState() const
		{
			return m_nState;
		}
		/*inline CVector3* GetNormals(int i)
		{
			return &(m_vNormals[i]);
		}*/
		inline const float* GetTexCoordsU() const
		{
			return (const float*)m_fTexCoords;
		}
		inline const float* GetTexCoordsV() const
		{
			return (const float*)(m_fTexCoords[1]);
		}
		inline const CRGBA* GetColor()
		{
			return m_rgbaColor;
		}
		inline void SetColor(const CRGBA& clr1, const CRGBA& clr2, const CRGBA& clr3)
		{
			m_rgbaColor[0] = clr1;
			m_rgbaColor[1] = clr2;
			m_rgbaColor[2] = clr3;
		}
	protected:
		unsigned int m_nState;
		unsigned short m_usVertIndices[3];	// Vertex indice
		//CVector3 m_vNormals[3];			// Vertex normals
		float m_fTexCoords[2][3];			// Texture coordicates, order: u1,u2,u3,v1,v2,v3

		//////////////////////////////////////////////////////////////////////////
	public:
		CRGBA		m_rgbaColor[3];			// Color of polygon or vertex, for lighting
		CRGBA		m_rgbaSpecular[3];		// Specular for lighting
		CMaterial*	m_pMaterial;			// Pointer to material
		CVertex4*	m_pVertList;			// Vertex list
	};
#pragma pack(pop, packing)
}


#endif //POLYGON_H_4A795054_795A_4E45_B9E3_9C67C41C18BC
