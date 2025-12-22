// scene.cpp: implementation of the CScene class.
//
//////////////////////////////////////////////////////////////////////

#include "scene.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace Nomad3D
{
	CScene::CScene()
	{
		m_pCull = new CCullOutSphere();
		m_RenderList.Init();
	}

	CScene::~CScene()
	{
		if(m_pCull)
		{
			delete m_pCull;
			m_pCull = NULL;
		}

		m_ObjectList.clear();
		m_matObjList.clear();
	}

	void CScene::AddObject(CObject* pobj, const CMatrix4& mat)
	{
		m_ObjectList.push_back(pobj);
		m_matObjList.push_back(mat);
	}

 	void CScene::SetCamera(const CCamera& cam)
 	{
 		m_Camera = cam;
 	}

	void CScene::ConvertObject2RenderList()
	{
		m_RenderList.Init();

		//int nNumObjects = m_ObjectList.size();
		CObjectList::iterator it_end = m_ObjectList.end();
		CObject2SceneMatrixList::iterator itMat=m_matObjList.begin();
		CObject* pobj=NULL;
		for(CObjectList::iterator it = m_ObjectList.begin(); it != it_end; it++, itMat++)
		{
			pobj = *it;
			if(m_pCull)
			{
				if (NM3D_OUT_OF_FRUSTUM == m_pCull->TestObject(pobj,&m_Camera))
					continue;
			}
			
			m_RenderList.InsertObject(pobj, &(*itMat));
		}
		m_RenderList.BuildMemory();
	}

	void CScene::LightScene(CLight* lt,CCamera* cam)
	{
		CObject* pobj=NULL;
		CObjectList::iterator it_end = m_ObjectList.end();
		for(CObjectList::iterator it = m_ObjectList.begin(); it != it_end; it++)
		{
			pobj = *it;
			lt->LightObject16(pobj,cam);
		}
	}

	void CScene::HideBackRenderList()
	{
		int n = m_RenderList.m_unNumPolys;
		CVertex4* pVertList = m_RenderList.m_pVertList;
		//CPolygon* pPoly = NULL;
		const unsigned short* pIndices = NULL;
		CVector4 v1,v2,cv,ce;
		for(int i=0; i<n; i++)
		{
			pIndices = m_RenderList.m_pPolyList[i].GetVertIndices();
			v1 = pVertList[pIndices[0]] - pVertList[pIndices[1]];
			v2 = pVertList[pIndices[2]] - pVertList[pIndices[1]];
			cv = Cross4(v1,v2);
			ce = m_Camera.GetPosition() - pVertList[pIndices[1]];
			if(Dot4(ce,cv) >= 0)
			{
				m_RenderList.m_pPolyList[i].AddState(NM3D_BACK_POLYGON);
			}
		}
	}
}
