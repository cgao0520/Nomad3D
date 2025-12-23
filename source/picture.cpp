// picture.cpp: implementation of the CPicture class.
//
//////////////////////////////////////////////////////////////////////

#include "picture.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "render.h"

namespace Nomad3D
{
	CPicture::CPicture()
	{
		Init();
	}

	void CPicture::Init()
	{
		//m_ViewInfo;
		//m_Camera;
		m_pScene = NULL;

		m_cBGTrans = 0;

		GeneratePictureMatrix();
	}

	void CPicture::GeneratePictureMatrix()
	{
		float a = (0.5f * m_ViewInfo.GetViewWidth());
		float b = (0.5f * m_ViewInfo.GetViewHeight());
		CMatrix4 ms
			(
			a,		0,		0,	0,
			0,		-b,		0,	0,
			0,		0,		1,	0,
			a+m_ViewInfo.GetStartX(),	b+m_ViewInfo.GetStartY(),	0,	1
			);
		
		m_matPictureMatrix = ms;		
	}

	CMatrix4 CPicture::GetPictureMatrix()
	{
		return m_matPictureMatrix;
	}

	void CPicture::Assign(const CViewInfo& vi, const CScene* scn)
	{
		m_ViewInfo = vi;
		m_pScene = (CScene*)scn;

		GeneratePictureMatrix();
	}
	
	CPicture::CPicture(const CViewInfo& vi, const CScene* scn)
	{
		Assign(vi,scn);
	}

	CPicture& CPicture::operator = (const CPicture& pic)
	{
		Assign(pic.m_ViewInfo, pic.m_pScene);
		return *this;
	}

	CPicture::~CPicture()
	{
		m_pScene = NULL;
	}

	void CPicture::SetViewInfo(const CViewInfo& vi)
	{
		m_ViewInfo = vi;
		GeneratePictureMatrix();
	}

	void CPicture::SetScene(const CScene* scn)
	{
		m_pScene = (CScene*)scn;
	}

	void CPicture::SetCamera(const CCamera& cam)
	{
		m_Camera = cam;
	}

	void CPicture::SetBGTrans(unsigned char a)
	{
		m_cBGTrans = a;
	}

	void CPicture::ResetTimer()
	{
		m_Timer.Init();
		m_Timer.GetSeconds();
	}

	//draw a picture on the screen!!!
	void CPicture::Draw(int n)
	{
		//render.SetRenderType(CRender::NM3D_RENDER_TYPE_FLAT);
		render.SetClipBox(m_ViewInfo.GetClipBox());
		render.SetClearColor(m_ViewInfo.GetClearColor());

		CRender::ERenderType eRT = render.GetRenderType();

		CMatrix4 mT(eIdentity),mT2(eIdentity);
		static int ang=0, ang_l=0;
		ang++;
		ang_l+=2;
		if(ang>=360)
			ang=0;
		if(ang_l>=360)
			ang_l=0;
		CMatrix4::Rotate(0,ang,0,mT);
		CMatrix4 m =mT;
		CMatrix4::Translate(0,0,30,mT);
		m*=mT;
		CMatrix4::Scale(1,1,1,mT);
		m*=mT;
#if 0
		CRenderList& renderlist = m_pScene->m_RenderList;

		m_pScene->ConvertObject2RenderList();

		renderlist.MultMatrix4(m);
		
		light.LightRenderList16(&renderlist,&m_pScene->m_Camera);

		if(render.GetRenderType() != CRender::NM3D_RENDER_TYPE_LINE)
			m_pScene->HideBackRenderList();

		m=m_pScene->m_Camera.GetCameraMatrix();
		m*=m_matPictureMatrix;
		renderlist.MultMatrix4(m);
		renderlist.DivideW();
		renderlist.Sort(enSortByAvgZ);

		CMatrix4::Rotate(0,ang_l,0,mT2);
		//light.Transform(mT2);
		//��Ҫдһ����RenderListִ�й��յĺ���
		//m_pScene->LightScene(&light,&m_pScene->m_Camera);
#else
		m_pScene->ConvertObject2RenderList();
		CRenderList& renderlist = *(m_pScene->GetRenderList());
		
		renderlist.MultMatrix4(m);
		if(render.GetRenderType() != CRender::NM3D_RENDER_TYPE_LINE)
			m_pScene->HideBackRenderList();
		
		CMatrix4 mLight;
		CMatrix4::Translate(20,10,-5,mLight);
		mLight *= m_pScene->GetCamera()->GetUVNMatrix();
		light.Transform(mLight);
		//light.LightRenderList16(&renderlist, m_pScene->GetCamera());
		
		//renderlist.MultMatrix4(m_pScene->GetCamera()->GetCameraMatrix());
		renderlist.MultMatrix4(m_pScene->GetCamera()->GetUVNMatrix());

		// View Frustum Culling
		int nCulledPolygons = 0;
		nCulledPolygons = renderlist.CullAndClip(m_pScene->GetCamera());
		
		light.LightRenderList16(&renderlist, m_pScene->GetCamera());
		
		renderlist.MultMatrix4(m_pScene->GetCamera()->GetPerspectiveMatrix()*m_matPictureMatrix);
		renderlist.DivideW();
		renderlist.Sort(enSortByAvgZ);
#endif
		//int nNumVerts = renderlist.GetNumVerts();
		int nNumPolys = renderlist.GetNumPolys();
		const CVertex4* pVertList = renderlist.GetVertList();
		//const CPolygon* pPolyList = renderlist.GetPolyList();
		CPolygon** ppL = renderlist.GetSortedPPL();
		const unsigned short* pIndices = NULL;
		
		render.SetPictureBGTrans(m_cBGTrans);
		if(n==0)
			render.ClearVBuffer();
		else
			render.ClearPictureBuffer();

		//return ;
		int i=0;
		float p[3][3];
		CMaterial* pMaterial = NULL;
		const CPolygon* pPoly = NULL;
		int rendered_polygons = 0;
		for(i=0; i<nNumPolys; i++)
		{
			pPoly = ppL[i];

			if(pPoly->GetState()&NM3D_CULLED_POLYGON || pPoly->GetState()&NM3D_BACK_POLYGON)
				continue;

			pIndices = pPoly->GetVertIndices();
			render.SetColor(pPoly->m_rgbaColor);
			render.SetSpecularColor(pPoly->m_rgbaSpecular);
			pMaterial = pPoly->m_pMaterial;
			render.SetMaterial(pMaterial);
			
			if(render.GetRenderType() != CRender::NM3D_RENDER_TYPE_LINE)
			{
				CVector4 v1 = pVertList[pIndices[0]] - pVertList[pIndices[1]];
				CVector4 v2 = pVertList[pIndices[0]] - pVertList[pIndices[2]];
				CVector4 cv = Cross4(v1,v2);
				CVector4 ce = m_pScene->GetCamera()->GetPosition() - pVertList[pIndices[0]];
				if(Dot4(ce,cv)>0)
					continue;
			}
			
			for(int j=0; j<3; j++)
			{
				p[j][0]=pVertList[pIndices[j]].x();
				p[j][1]=pVertList[pIndices[j]].y();
				p[j][2]=pVertList[pIndices[j]].z();
			}
			
			render.DrawTriangle(p);
			rendered_polygons++;
		}

		char text[50];
		memset(text,0,50);
		sprintf(text,"rendered polygons: %d\n",rendered_polygons);
		render.DrawText(0,0,text);
		memset(text,0,50);
		sprintf(text,"culled polygons: %d\n",nCulledPolygons);
		render.DrawText(-1,-1,text);

		render.SetRenderType(eRT);
	}	
}
