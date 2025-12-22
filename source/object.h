// object.h: interface for the CObject class.
//
//////////////////////////////////////////////////////////////////////

#ifndef OBJECT_H_9A736E8A_0D59_432E_B6AF_62779327E3C6
#define OBJECT_H_9A736E8A_0D59_432E_B6AF_62779327E3C6


#include "vector3.h"
#include "simplelist.h"
#include "mesh.h"
#include "vertex.h"
#include "polygon.h"
#include "material.h"
#include "joint.h"

namespace Nomad3D
{
#pragma pack(push, packing)
#pragma pack(1)

	class CRenderList;
	class CLight;

	class CObject
	{
		friend class CRenderList;
		friend class CLight;
	public:
		CObject()
		{
			m_nId = 1;
			m_nState = 0;
			//m_vWorldPos = CPoint3(0,0,0);
			m_fMaxRadius = 0;

			m_unNumVerts = 0;
			m_unNumPolys = 0;
			m_unNumMeshes = 0;
			m_unNumMaterials = 0;
			m_unNumJoints = 0;

			m_pVertList = NULL;
			m_pVertListTran = NULL;
			m_pPolyList = NULL;
			m_pMeshList = NULL;
			m_pMaterialList = NULL;
			m_pJointList = NULL;
		};
		virtual ~CObject()
		{
			if(m_pVertList)
				delete[] m_pVertList;
			if(m_pVertListTran)
				delete[] m_pVertListTran;
			if(m_pPolyList)
				delete[] m_pPolyList;
			if(m_pMeshList)
				delete[] m_pMeshList;
			if(m_pMaterialList)
				delete[] m_pMaterialList;
			if(m_pJointList)
				delete[] m_pJointList;
		}
	public:
		CVertex4* GetVertList() const
		{
			return m_pVertList;
		}
		CVertex4* GetVertListTran() const
		{
			return m_pVertListTran;
		}
		unsigned int GetNumVerts() const 
		{
			return m_unNumVerts;
		}
		CJoint* GetJointList() const
		{
			return m_pJointList;
		}
		unsigned int GetNumJoints() const
		{
			return m_unNumJoints;
		}
		CPolygon* GetPolyList() const
		{
			return m_pPolyList;
		}
		unsigned int GetNumPolys() const
		{
			return m_unNumPolys;
		}

		const CPoint3& GetPos() const
		{
			return m_vWorldPos;
		}
		float GetMaxRadius() const
		{
			return m_fMaxRadius;
		}
		void ComputeRadius(bool div2 = true)
		{
			float length;
			for(unsigned int i=0; i<m_unNumVerts; i++)
			{
				length = m_pVertList[i].Length();
				if( m_fMaxRadius < length)
					m_fMaxRadius = length;
			}
			if(div2)
				m_fMaxRadius /= 2;
		}
	public:
		virtual bool ReadObjectFile(const char* sFilePath)=0;
		virtual bool SetupSkeleton() = 0;
	protected:
		int						m_nId;
		int						m_nState;
		CPoint3					m_vWorldPos;
		float					m_fMaxRadius;
		//Vertex
		unsigned int			m_unNumVerts;
		CVertex4*				m_pVertList;
		CVertex4*				m_pVertListTran;//use to transform
		//Polygon
		unsigned int			m_unNumPolys;
		CPolygon*				m_pPolyList;
		//Mesh
		unsigned int			m_unNumMeshes;
		CMesh*					m_pMeshList;
		//Material
		unsigned int			m_unNumMaterials;
		CMaterial*				m_pMaterialList;
		//Joint
		unsigned int			m_unNumJoints;
		CJoint*					m_pJointList;
	};
#pragma pack(pop, packing)
}


#endif // OBJECT_H_9A736E8A_0D59_432E_B6AF_62779327E3C6
