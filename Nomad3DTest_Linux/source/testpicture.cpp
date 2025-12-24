// testpicture.cpp: implementation of the CTestPicture class.
//
//////////////////////////////////////////////////////////////////////

#include "testpicture.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTestPicture::CTestPicture()
{

}

CTestPicture::~CTestPicture()
{
	m_ObjectList.clear();
}

//////////////////////////////////////////////////////////////////////////

void CTestPicture::Draw(int n)
{
	CRender::ERenderType eRT = render.GetRenderType();
/*
	float a = (0.5f * m_ViewInfo.GetViewWidth())-0.5;
	float b = (0.5f * m_ViewInfo.GetViewHeight())-0.5;
	CMatrix4 ms
		(
		a,		0,		0,	0,
		0,		-b,		0,	0,
		0,		0,		1,	0,
		a+m_ViewInfo.GetStartX()+0.5,	b+m_ViewInfo.GetStartY()+0.5,	0,	1
		);
*/
	
	render.SetClipBox(m_ViewInfo.GetClipBox());
	render.SetClearColor(m_ViewInfo.GetClearColor());
	
	
	CMatrix4 mT(eIdentity),mT2(eIdentity);
	static int ang=360, ang_l=0;
	ang--;
	ang_l+=2;
	if(ang<=0)
		ang=360;
	if(ang_l>=360)
		ang_l=0;
	CMatrix4::Rotate(0,ang,0,mT);
	CMatrix4 m =mT;
	CMatrix4::Translate(0,-30,-10,mT);
	//CMatrix4::Translate(0,-5,30,mT);
	m*=mT;
//////////////////////////////////////////////////////////////////////////
#if 1
	float fStartTime = 0.0f;
	float fEndTime = 2.0f;//9;//
	float fSpeed = 1.0f;

	CMS3DObject* pObject = (CMS3DObject*)(*(m_ObjectList.begin()));
	static bool bFirstTime = true;
	if(bFirstTime)
	{
		ResetTimer();
		bFirstTime = false;
	}
	
	static float fLastTime = fStartTime;
	
	float fTime = m_Timer.GetSeconds() * fSpeed;
	fTime += fLastTime;
	fLastTime = fTime;
	//////////////////////////////////////////////////////////////////////////
	//fTime = fStartTime = fLastTime = 0;
	//////////////////////////////////////////////////////////////////////////

	//looping
	bool bLoop = true;
	if(fTime > fEndTime)
	{
		if(bLoop)
		{
			ResetTimer();
			fLastTime = fStartTime;
			fTime = fStartTime;
		}
		else
			fTime = fEndTime;
	}

	//int nObjs=m_ObjectList.size();
	int nNumJoints = pObject->GetNumJoints();
	CJoint* pJointList = pObject->GetJointList();
	CJoint* pJoint = NULL;
	unsigned int uiFrame = 0;
	CMatrix4 matTran;
	CMatrix4 matRot;
	CMatrix4 matFinal;
	
	for(int i=0; i<nNumJoints; i++)
	{
		pJoint = &(pJointList[i]);
		uiFrame = 0;

		if(pJoint->m_usNumRotFrames == 0 && pJoint->m_usNumTransFrames == 0)
		{
			pJoint->m_matFinal = pJoint->m_matAbs;
			continue;
		}

		//!!!!!!!!!!!!!!!!!!!!!!Translation!!!!!!!!!!!!!!!!!!!!!!
		while(uiFrame < pJoint->m_usNumTransFrames && pJoint->m_pTransKeyFrames[uiFrame].m_fTime < fTime)
			uiFrame++;
		pJoint->m_usCurTransFrame = uiFrame;

		float fTranslation[3];
		float fDeltaT = 1;
		float fInterp = 0;

		if(uiFrame == 0)
			memcpy(fTranslation, pJoint->m_pTransKeyFrames[0].m_fParam, sizeof(float[3]));
		else if(uiFrame == pJoint->m_usNumTransFrames)
			memcpy(fTranslation, pJoint->m_pTransKeyFrames[uiFrame-1].m_fParam, sizeof(float[3]));
		else
		{
			CKeyFrame * pkCur = &pJoint->m_pTransKeyFrames[uiFrame];
			CKeyFrame * pkPrev = &pJoint->m_pTransKeyFrames[uiFrame-1];
			
			fDeltaT = pkCur->m_fTime - pkPrev->m_fTime;
			fInterp = (fTime - pkPrev->m_fTime) / fDeltaT;
			
			fTranslation[0] = pkPrev->m_fParam[0] + (pkCur->m_fParam[0] - pkPrev->m_fParam[0]) * fInterp;
			fTranslation[1] = pkPrev->m_fParam[1] + (pkCur->m_fParam[1] - pkPrev->m_fParam[1]) * fInterp;
			fTranslation[2] = pkPrev->m_fParam[2] + (pkCur->m_fParam[2] - pkPrev->m_fParam[2]) * fInterp;
		}
	
		CMatrix4::Translate(fTranslation[0],fTranslation[1],fTranslation[2],matTran);

		//!!!!!!!!!!!!!!!!!!!!!!Rotation!!!!!!!!!!!!!!!!!!!!!!
		uiFrame = 0;
		while(uiFrame < pJoint->m_usNumRotFrames && pJoint->m_pRotKeyFrames[uiFrame].m_fTime < fTime)
			uiFrame++;

		if(uiFrame == 0)
			CMatrix4::Rotate(
			R2D(pJoint->m_pRotKeyFrames[0].m_fParam[0]), 
			R2D(pJoint->m_pRotKeyFrames[0].m_fParam[1]), 
			R2D(pJoint->m_pRotKeyFrames[0].m_fParam[2]), matRot);
		else if(uiFrame == pJoint->m_usNumTransFrames)
			CMatrix4::Rotate(
			R2D(pJoint->m_pRotKeyFrames[uiFrame-1].m_fParam[0]), 
			R2D(pJoint->m_pRotKeyFrames[uiFrame-1].m_fParam[1]), 
			R2D(pJoint->m_pRotKeyFrames[uiFrame-1].m_fParam[2]), matRot);
		else
		{
			CKeyFrame * pkCur = &pJoint->m_pRotKeyFrames[uiFrame];
			CKeyFrame * pkPrev = &pJoint->m_pRotKeyFrames[uiFrame-1];
			
			fDeltaT = pkCur->m_fTime - pkPrev->m_fTime;
			fInterp = (fTime - pkPrev->m_fTime) / fDeltaT;
			
			//Create a rotation quaternion for each frame
			CQuaternion qCur;
			CQuaternion qPrev;
			qCur.FromEulers(pkCur->m_fParam[0], pkCur->m_fParam[1], pkCur->m_fParam[2]);
			qPrev.FromEulers(pkPrev->m_fParam[0], pkPrev->m_fParam[1], pkPrev->m_fParam[2]);

			//SLERP between the two frames
			CQuaternion qFinal = SLERP(qPrev, qCur, fInterp);
			
			//Convert the quaternion to a rotation matrix
			matRot = qFinal.ToMatrix4();
		}

		matFinal = matRot;
		matFinal *= matTran;
		matFinal *= pJoint->m_matLocal;

		if(pJoint->m_nParent == NM3D_JOINT_NO_PARENT)
			pJoint->m_matFinal = matFinal;
		else
			pJoint->m_matFinal = matFinal * pJointList[pJoint->m_nParent].m_matFinal;

	}

	unsigned int nNVt = pObject->GetNumVerts();
	CVertex4* pVtList = pObject->GetVertList();
	CVertex4* pVtListTran = pObject->GetVertListTran();
	CVertex4* pVt;
	CVertex4 vt;
	for(unsigned int i=0; i<nNVt; i++)
	{
		pVt = &(pVtList[i]);
		
		if(pVt->GetBone() != NM3D_JOINT_NO_PARENT)
		{
			pJoint = &(pJointList[(int)pVt->GetBone()]);
			pVtListTran[i] = (*pVt) * pJoint->m_matFinal;
		}
		else
		{
			pVtListTran[i] = *pVt;
		}
	}
#endif
//////////////////////////////////////////////////////////////////////////
	
	m_pScene->ConvertObject2RenderList();
	CRenderList& renderlist = *(m_pScene->GetRenderList());
	
	renderlist.MultMatrix4(m);
	if(render.GetRenderType() != CRender::NM3D_RENDER_TYPE_LINE)
		m_pScene->HideBackRenderList();

	CMatrix4 mLight;
	CMatrix4::Translate(20,10,-5,mLight);
	mLight *= m_pScene->GetCamera()->GetUVNMatrix();
 	light.Transform(mLight/*m*/);
 	//light.LightRenderList16(&renderlist, m_pScene->GetCamera());
	
	//renderlist.MultMatrix4(m_pScene->GetCamera()->GetCameraMatrix());
	renderlist.MultMatrix4(m_pScene->GetCamera()->GetUVNMatrix());

	// View Frustum Clipping
	int nCulledPolygons = 0;
	nCulledPolygons = renderlist.CullAndClip(m_pScene->GetCamera());

	light.LightRenderList16(&renderlist, m_pScene->GetCamera());

	renderlist.MultMatrix4(m_pScene->GetCamera()->GetPerspectiveMatrix()*m_matPictureMatrix);
	renderlist.DivideW();

	if(!render.GetZBufferState())
		renderlist.Sort(enSortByAvgZ);

	//m_pScene->LightScene(&light,&m_pScene->m_Camera);
	
	//int nNumVerts = renderlist.GetNumVerts();
	int nNumPolys = renderlist.GetNumPolys();
	const CVertex4* pVertList = renderlist.GetVertList();
	CPolygon** ppL = renderlist.GetSortedPPL();
	const unsigned short* pIndices = NULL;
	
	render.SetPictureBGTrans(255);
	if(n==0)
		render.ClearVBuffer();
	else
		render.ClearPictureBuffer();
	
	//return ;
	float p[3][3];
	float u[3],v[3];
	CMaterial* pMaterial = NULL;
	/*const*/ CPolygon* pPoly = NULL;

	int rendered_polygons = 0;
	for(int i=0; i<nNumPolys; i++)
	{
		pPoly = ppL[i];

		if(pPoly->GetState()&NM3D_CULLED_POLYGON || pPoly->GetState()&NM3D_BACK_POLYGON)
			continue;

		pIndices = pPoly->GetVertIndices();
		render.SetColor(pPoly->m_rgbaColor);
		render.SetSpecularColor(pPoly->m_rgbaSpecular);
		pMaterial = pPoly->m_pMaterial;
		render.SetMaterial(pMaterial);
/*		
		if(render.GetRenderType() != CRender::NM3D_RENDER_TYPE_LINE)
		{
			CVector4 v1 = pVertList[pIndices[0]] - pVertList[pIndices[1]];
			CVector4 v2 = pVertList[pIndices[0]] - pVertList[pIndices[2]];
			CVector4 cv = Cross4(v1,v2);
			CVector4 ce = m_pScene->GetCamera()->GetPosition() - pVertList[pIndices[0]];
			if(Dot4(ce,cv)>0)
				continue;
		}
*/		
		for(int j=0; j<3; j++)
		{
			p[j][0]=pVertList[pIndices[j]].x();
			p[j][1]=pVertList[pIndices[j]].y();
			p[j][2]=pVertList[pIndices[j]].z();
		}
		
		const float* cu = pPoly->GetTexCoordsU();
		const float* cv = pPoly->GetTexCoordsV();
		int nHeight = pMaterial->m_imgTexture.GetHeight();
		int nWidth = pMaterial->m_imgTexture.GetWidth();
		u[0]=cu[0]*nWidth;u[1]=cu[1]*nWidth;u[2]=cu[2]*nWidth;
		v[0]=cv[0]*nHeight;v[1]=cv[1]*nHeight;v[2]=cv[2]*nHeight;
		render.SetTextureCoords(u,v);
		
		render.DrawTriangle(p);
		rendered_polygons++;
	}
	
	static u8 rr=150;
	rr++;
	if(rr>=255)
		rr = 150;
	render.SetTextColor(CRGBA(rr,rr,rr,1));

	render.DrawText(0,0,"\n\tTHIS IS A Nomad3D DEMO, MADE BY c_gao\n\n");
	render.DrawText(-1,-1,"\n\tWelcome to Nomad3D's world!");
	render.DrawText(-1,-1,"\n\tPress 'F' to FLAT SHADING mode!");
	render.DrawText(-1,-1,"\n\tPress 'G' to GOURAUD SHADING mode!");
	render.DrawText(-1,-1,"\n\tPress 'H' to GOURAUD SHADING mode(3D master function)!");
	render.DrawText(-1,-1,"\n\tPress 'T' to GOURAUD TEXTURE mode!");
	render.DrawText(-1,-1,"\n\tPress 'Y' to GOURAUD TEXTURE mode(3D master function)!");
	render.DrawText(-1,-1,"\n\tPress 'B' to LINE WITH BACK LINE HIDDEN mode!");
	render.DrawText(-1,-1,"\n\tPress 'L' to LINE WITHOUT BACK LINE HIDDEN mode!");
	render.DrawText(-1,-1,"\n\tPress 'A' to YAW to left!");
	render.DrawText(-1,-1,"\n\tPress 'D' to YAW to right!");
	render.DrawText(-1,-1,"\n\tPress 'Z' to Turn ON/OFF Z buffer!");
	render.DrawText(-1,-1,"\n\tPress 'X' to Turn ON/OFF Light 0 (Direction)!");
	render.DrawText(-1,-1,"\n\tPress 'C' to Turn ON/OFF Light 1 (Point)!");
	render.DrawText(-1,-1,"\n\tPress 'V' to Turn ON/OFF Light 2 (Spot)!");
	render.DrawText(-1,-1,"\n\tPress Direction button to move camera!");

	char text[50];
	memset(text,0,50);
	sprintf(text,"\n\n\tTotal polygons: %d\n",nNumPolys);
	render.DrawText(-1,-1,text);
	memset(text,0,50);
	sprintf(text,"\tRendered polygons: %d\n",rendered_polygons);
	render.DrawText(-1,-1,text);
	memset(text,0,50);
	sprintf(text,"\tCulled polygons: %d\n",nCulledPolygons);
	render.DrawText(-1,-1,text);

	render.SetRenderType(eRT);
}