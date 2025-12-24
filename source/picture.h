// picture.h: interface for the CPicture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(PICTURE_H_C8031D48_3848_47D8_96A6_1357BE5BC06E)
#define PICTURE_H_C8031D48_3848_47D8_96A6_1357BE5BC06E


#include "camera.h"
#include "scene.h"
#include "render.h"
#include "rgb.h"
#include "timer.h"
#include "view.h"

namespace Nomad3D
{
	class CPicture  
	{
	public:
		CPicture();
		CPicture(const CViewInfo& vi, const CScene* scn);
		virtual ~CPicture();
		void Init();
		void Assign(const CViewInfo& vi, const CScene* scn);
		CPicture& operator = (const CPicture& pic);
		//////////////////////////////////////////////////////////////////////////
		CMatrix4 GetPictureMatrix();
		void SetViewInfo(const CViewInfo& vi);
		void SetScene(const CScene* scn);
		void SetCamera(const CCamera& cam);
		void SetBGTrans(unsigned char a=0);
		//////////////////////////////////////////////////////////////////////////
		virtual void Draw(int n=0);/*the most important function!!!*/
	protected:
		void GeneratePictureMatrix();
	protected:
		CViewInfo	m_ViewInfo;
		CScene*		m_pScene;
		CMatrix4	m_matPictureMatrix;	//Perspective to View
		unsigned char m_cBGTrans; //Background Transparency
	};
}


#endif // !defined(PICTURE_H_C8031D48_3848_47D8_96A6_1357BE5BC06E)
