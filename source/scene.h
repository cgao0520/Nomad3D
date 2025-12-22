// scene.h: interface for the CScene class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SCENE_H_30CC8371_FA50_4A14_83AD_4D5336D60109)
#define SCENE_H_30CC8371_FA50_4A14_83AD_4D5336D60109


#include "object.h"
#include "light.h"
#include "simplelist.h"
#include "renderlist.h"
#include "camera.h"
#include "cull.h"
#include <list>

namespace Nomad3D
{
	class CPicture;

	class CScene  // World
	{
		typedef std::list<CObject*>	CObjectList;
		typedef std::list<CMatrix4> CObject2SceneMatrixList;

		friend class CPicture;
	public:
		CScene();
		virtual ~CScene();
		void AddObject(CObject* pobj, const CMatrix4& mat = CMatrix4(eIdentity));
		//void AddLight();
		void SetCamera(const CCamera& cam);
		void LightScene(CLight* lt,CCamera* cam);
		void HideBackRenderList();

		//////////////////////////////////////////////////////////////////////////
		//Get Method
		inline CRenderList* GetRenderList()
		{
			return &m_RenderList;
		}
		inline CCamera* GetCamera()
		{
			return &m_Camera;
		}
	//protected:
		void ConvertObject2RenderList();
	protected:
		//CSimpleList<CObject*>	m_ObjectList;
		CObjectList				m_ObjectList;
		CObject2SceneMatrixList	m_matObjList;
		CRenderList				m_RenderList;
		CCamera					m_Camera;
		//////////////////////////////////////////////////////////////////////////
		CCull*					m_pCull;
	};
}

#endif // !defined(SCENE_H_30CC8371_FA50_4A14_83AD_4D5336D60109)
