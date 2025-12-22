// renderlist.h: interface for the CRenderList class.
//
//////////////////////////////////////////////////////////////////////

#ifndef RENDERLIST_H_400560F4_7E29_4AD2_BAE4_9927952F863F
#define RENDERLIST_H_400560F4_7E29_4AD2_BAE4_9927952F863F

#include "object.h"
#include "vertex.h"
#include "matrix4.h"
#include "polygon.h"
#include "clipbox.h"
#include "camera.h"
#include <list>

namespace Nomad3D
{
	enum ESortType
	{
		enSortByAvgZ,
		enSortByMaxZ,
		enSortByMinZ
	};

	class CScene;
	class CRender;
	class CObject;

	class CRenderList  
	{
		typedef std::list<CObject*>		CObjectList;
		typedef std::list<CMatrix4*>	CObject2SceneMatrixList;
		friend class CScene;
		friend class CRender;
	public:
		CRenderList();
		virtual ~CRenderList();
	public:
		void Init(bool bClearCache=false);
		void InsertObject(CObject* pObj, CMatrix4* pMat=NULL, bool bCalcMem = false);
		void MultMatrix4(const CMatrix4& m);
		void DivideW();
		unsigned int GetNumVerts() const;
		unsigned int GetNumPolys() const;
		const CVertex4* GetVertList();
		CPolygon* GetPolyList();
		void Sort(ESortType enST);
		int CullAndClip(const CCamera* pCam);
		//////////////////////////////////////////////////////////////////////////
		CPolygon** GetSortedPPL();
		//////////////////////////////////////////////////////////////////////////
	protected:
		void BuildMemory();
		static int CompareAvgZ(const void *arg1, const void *arg2);
		static int CompareMinZ(const void *arg1, const void *arg2);
		static int CompareMaxZ(const void *arg1, const void *arg2);

	protected:
		CObjectList				m_ObjectList;
		CObject2SceneMatrixList	m_matObjMatrixList;

		unsigned int			m_unNumVerts;
		CVertex4*				m_pVertList;
		unsigned int			m_unNumPolys;
		CPolygon*				m_pPolyList;
		//////////////////////////////////////////////////////////////////////////
		CPolygon**				m_ppPL;
	};
}


#endif // RENDERLIST_H_400560F4_7E29_4AD2_BAE4_9927952F863F
