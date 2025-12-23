// renderlist.cpp: implementation of the CRenderList class.
//
//////////////////////////////////////////////////////////////////////

#include "renderlist.h"
//#include "polygon.h"

#if NM3D_DEBUG_ON
	#include <assert.h>
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace Nomad3D
{
	CRenderList::CRenderList()
	{
		//////////////////////////////////////////////////////////////////////////
		m_ppPL = NULL;
		//////////////////////////////////////////////////////////////////////////
		m_pVertList = NULL;
		m_pPolyList = NULL;
		Init();
	}
	
	CRenderList::~CRenderList()
	{
		if(m_pVertList)
			free(m_pVertList);
		if(m_pPolyList)
			free(m_pPolyList);
		//////////////////////////////////////////////////////////////////////////
		if(m_ppPL)
			free(m_ppPL);
		//////////////////////////////////////////////////////////////////////////

		m_matObjMatrixList.clear();
	}

	void CRenderList::Init(bool bClearCache)
	{
		m_unNumVerts = 0;
		m_unNumPolys = 0;
		m_ObjectList.clear();

		if(bClearCache)
		{
			if(m_pVertList)
				free(m_pVertList);
			if(m_pPolyList)
				free(m_pPolyList);
			if(m_ppPL)
				free(m_ppPL);
			m_pVertList = NULL;
			m_pPolyList = NULL;
			m_ppPL = NULL;
		}
	}

	void CRenderList::InsertObject(CObject* pObj, CMatrix4* pMat/*=NULL*/, bool bCalcMem/* = false*/)
	{
		m_ObjectList.push_back(pObj);
		m_matObjMatrixList.push_back(pMat);

		if(bCalcMem)
			BuildMemory();
	}

	void CRenderList::BuildMemory()
	{
		unsigned int nv=0,np=0;
		unsigned int NV=0,NP=0;
		CObject* pObj = NULL;
		CObjectList::iterator it_end = m_ObjectList.end();
		CObjectList::iterator it;
		CObject2SceneMatrixList::iterator itMatrix;
		//CObject2SceneMatrixList::iterator itMatrix_end;
		CMatrix4 matIdentity(eIdentity);
		int sizeV=sizeof(CVertex4);
		int sizeP=sizeof(CPolygon);
		for(it = m_ObjectList.begin(); it != it_end; it++)
		{
			pObj = (*it);
			NV += pObj->m_unNumVerts;
			NP += pObj->m_unNumPolys;
		}

		if(m_unNumVerts < NV)
		{
			if(m_pVertList)
				free(m_pVertList);
			m_pVertList = (CVertex4*)malloc(NV*sizeV);
			m_unNumVerts = NV;
		}

		if(m_unNumPolys < NP)
		{
			if(m_pPolyList)
				free(m_pPolyList);
			m_pPolyList = (CPolygon*)malloc(NP*sizeP);
			//////////////////////////////////////////////////////////////////////////
			if(m_ppPL)
				free(m_ppPL);
			m_ppPL = (CPolygon**)malloc(NP*sizeof(CPolygon*));
			//////////////////////////////////////////////////////////////////////////
			m_unNumPolys = NP;
		}

		//////////////////////////////////////////////////////////////////////////
		for(unsigned int _i=0; _i<NP; _i++)
		{
			m_ppPL[_i] = m_pPolyList + _i;
		}
		//////////////////////////////////////////////////////////////////////////
		
		if(m_ObjectList.size() > 0)
		{
			it = m_ObjectList.begin();
			
			pObj = (*it);
			if(pObj != NULL)
			{
				//memcpy(m_pVertList, pObj->m_pVertListTran, pObj->m_unNumVerts*sizeV);
				for (unsigned int i = 0; i < pObj->m_unNumVerts; i++)
				{
					m_pVertList[i] = pObj->m_pVertListTran[i];
				}
				nv = pObj->m_unNumVerts;
				//memcpy(m_pPolyList, pObj->m_pPolyList, pObj->m_unNumPolys*sizeP);
				for (unsigned int i = 0; i < pObj->m_unNumPolys; i++)
				{
					m_pPolyList[i] = pObj->m_pPolyList[i];
				}
				np = pObj->m_unNumPolys;

				for(unsigned int i=0; i<np; i++)
				{
					m_pPolyList[i].m_pVertList = m_pVertList;
				}
				
				itMatrix = m_matObjMatrixList.begin();
				CMatrix4* pMat = *itMatrix;
				if(pMat != NULL && *pMat != matIdentity)
				{
					for(unsigned int i=0; i<nv; i++)
					{
						m_pVertList[i] *= *pMat;
					}
				}

			}

			for(it++, itMatrix++; it != it_end; it++, itMatrix++)
			{
				pObj = (*it);
				//memcpy(m_pVertList+nv, pObj->m_pVertListTran, pObj->m_unNumVerts*sizeV);
				//memcpy(m_pPolyList+np, pObj->m_pPolyList, pObj->m_unNumPolys*sizeP);
				for (unsigned int i = 0; i < pObj->m_unNumVerts; i++)
				{
					m_pVertList[nv + i] = pObj->m_pVertListTran[i];
				}
				for (unsigned int i = 0; i < pObj->m_unNumPolys; i++)
				{
					m_pPolyList[np + i] = pObj->m_pPolyList[i];
				}

				for(unsigned int i=0; i<pObj->m_unNumPolys; i++)
				{
					m_pPolyList[np+i].m_usVertIndices[0] += nv;
					m_pPolyList[np+i].m_usVertIndices[1] += nv;
					m_pPolyList[np+i].m_usVertIndices[2] += nv;

					m_pPolyList[np+i].m_pVertList = m_pVertList;
				}
				np += pObj->m_unNumPolys;
				nv += pObj->m_unNumVerts;

				CMatrix4* pMat = *itMatrix;
				if(pMat != NULL && *pMat != matIdentity)
				{
					for(unsigned int i=nv; i<nv+pObj->m_unNumVerts; i++)
					{
						m_pVertList[i] *= *pMat;
					}
				}
			}//for it++
		}//if(m_ObjectList.size() > 0)
	}//end function

	void CRenderList::MultMatrix4(const CMatrix4& m)
	{
		unsigned int i=0;
		for(; i<m_unNumVerts; i++)
		{
			m_pVertList[i] *= m;
/*
			if(m_pVertList[i][0]>1)
			{
				int a;
				a=10;
			}
*/
		}

	}

	void CRenderList::DivideW()
	{
		CVertex4* pVert = NULL;
		float w;
		for(unsigned int i=0; i<m_unNumVerts; i++)
		{
			pVert = m_pVertList+i;
			w = pVert->v_[3];
			pVert->v_[0] /= w;
			pVert->v_[1] /= w;
			pVert->v_[2] /= w;
			//pVert->v_[3] = 1;
		}
	}

	unsigned int CRenderList::GetNumVerts() const
	{
		return m_unNumVerts;
	}

	unsigned int CRenderList::GetNumPolys() const
	{
		return m_unNumPolys;
	}

	const CVertex4* CRenderList::GetVertList()
	{
		return m_pVertList;
	}

	CPolygon* CRenderList::GetPolyList()
	{
		return m_pPolyList;
	}
	//////////////////////////////////////////////////////////////////////////
	CPolygon** CRenderList::GetSortedPPL()
	{
		return m_ppPL;
	}
	//////////////////////////////////////////////////////////////////////////

	CVertex4*		_Sort_VertList_;

	void CRenderList::Sort(ESortType enST)
	{
		_Sort_VertList_ = m_pVertList;
		switch(enST)
		{
		case enSortByAvgZ:
			qsort((void *)m_ppPL, m_unNumPolys, sizeof(CPolygon*), CompareAvgZ);
			break;
		case enSortByMaxZ:
			qsort((void *)m_ppPL, m_unNumPolys, sizeof(CPolygon*), CompareMaxZ);
			break;
		case enSortByMinZ:
			qsort((void *)m_ppPL, m_unNumPolys, sizeof(CPolygon*), CompareMinZ);
			break;
		}
	}

	int CRenderList::CompareAvgZ(const void *arg1, const void *arg2)
	{
		float z1, z2;
		
		CPolygon* poly_1 = *((CPolygon**)(arg1));
		CPolygon* poly_2 = *((CPolygon**)(arg2));
		
		z1 = //(float)0.33333*(
			_Sort_VertList_[poly_1->m_usVertIndices[0]].z() + \
			_Sort_VertList_[poly_1->m_usVertIndices[1]].z() + \
			_Sort_VertList_[poly_1->m_usVertIndices[2]].z();//);

		z2 = //(float)0.33333*(
			_Sort_VertList_[poly_2->m_usVertIndices[0]].z() + \
			_Sort_VertList_[poly_2->m_usVertIndices[1]].z() + \
			_Sort_VertList_[poly_2->m_usVertIndices[2]].z();//);
		
		if (z1 > z2)
			return(-1);
		else
			if (z1 < z2)
				return(1);
			else
				return(0);
	} 
	
	int CRenderList::CompareMinZ(const void *arg1, const void *arg2)
	{
		float z1, z2;
		
		CPolygon* poly_1 = *((CPolygon**)(arg1));
		CPolygon* poly_2 = *((CPolygon**)(arg2));
		
		z1 = MIN(_Sort_VertList_[poly_1->m_usVertIndices[0]].z(), _Sort_VertList_[poly_1->m_usVertIndices[1]].z());
		z1 = MIN(z1, _Sort_VertList_[poly_1->m_usVertIndices[2]].z());
		
		z2 = MIN(_Sort_VertList_[poly_2->m_usVertIndices[0]].z(), _Sort_VertList_[poly_2->m_usVertIndices[1]].z());
		z2 = MIN(z2, _Sort_VertList_[poly_2->m_usVertIndices[2]].z());
		
		if (z1 > z2)
			return(-1);
		else
			if (z1 < z2)
				return(1);
			else
				return(0);		
	}
	
	int CRenderList::CompareMaxZ(const void *arg1, const void *arg2)
	{
		float z1, z2;
		
		CPolygon* poly_1 = *((CPolygon**)(arg1));
		CPolygon* poly_2 = *((CPolygon**)(arg2));
		
		z1 = MAX(_Sort_VertList_[poly_1->m_usVertIndices[0]].z(), _Sort_VertList_[poly_1->m_usVertIndices[1]].z());
		z1 = MAX(z1, _Sort_VertList_[poly_1->m_usVertIndices[2]].z());
		
		z2 = MAX(_Sort_VertList_[poly_2->m_usVertIndices[0]].z(), _Sort_VertList_[poly_2->m_usVertIndices[1]].z());
		z2 = MAX(z2, _Sort_VertList_[poly_2->m_usVertIndices[2]].z());
		
		if (z1 > z2)
			return(-1);
		else
			if (z1 < z2)
				return(1);
			else
				return(0);		
	}

	int CRenderList::CullAndClip(const CCamera* pCam)
	{
#define CULLANDCLIP_GET_POINT_VALUES	\
	do{ \
	x1=pPoly->m_pVertList[pPoly->m_usVertIndices[v1]].x(); y1=pPoly->m_pVertList[pPoly->m_usVertIndices[v1]].y(); z1=pPoly->m_pVertList[pPoly->m_usVertIndices[v1]].z(); \
	x2=pPoly->m_pVertList[pPoly->m_usVertIndices[v2]].x(); y2=pPoly->m_pVertList[pPoly->m_usVertIndices[v2]].y(); z2=pPoly->m_pVertList[pPoly->m_usVertIndices[v2]].z(); \
	x3=pPoly->m_pVertList[pPoly->m_usVertIndices[v3]].x(); y3=pPoly->m_pVertList[pPoly->m_usVertIndices[v3]].y(); z3=pPoly->m_pVertList[pPoly->m_usVertIndices[v3]].z(); \
	}while(0)

		const float fXTan = Tan(pCam->GetFovy()*0.5);
		const float fYTan = Tan(pCam->GetFovy()*0.5)/pCam->GetAspect();
		const float min_z = -pCam->GetNear();
		const float max_z = -pCam->GetFar();

		
		int nCulledPolygons = 0;

		float x1,y1,z1,x2,y2,z2,x3,y3,z3;
		unsigned char r1,g1,b1,r2,g2,b2,r3,g3,b3;
		unsigned int i=0;
		int v1=0,v2=1,v3=2;
		float t1,t2;
		bool bTexture = false;
		CPolygon* pPoly = NULL;
		for(i=0; i<m_unNumPolys; i++)
		{
			v1=0,v2=1,v3=2;

			pPoly = m_pPolyList+i;

			if(pPoly->GetState() & NM3D_BACK_POLYGON)
				continue;

			CULLANDCLIP_GET_POINT_VALUES;

			if(
				(x1 <= fXTan*z1 && x2 <= fXTan*z2 && x3 <= fXTan*z3) ||	//1. left
				(x1 >= -fXTan*z1 && x2 >= -fXTan*z2 && x3 >= -fXTan*z3) ||	//2. right
				(y1 <= fYTan*z1 && y2 <= fYTan*z2 && y3 <= fYTan*z3) ||	//3. top
				(y1 >= -fYTan*z1 && y2 >= -fYTan*z2 && y3 >= -fYTan*z3) ||	//4. bottom
				(z1 >= min_z && z2 >= min_z && z3 >= min_z) ||	//5. near
				(z1 <= max_z && z2 <= max_z && z3 <= max_z)		//6. far
				)
			{
				//continue;
				pPoly->AddState(NM3D_CULLED_POLYGON);
				nCulledPolygons++;
				continue;
			}
			else if(z1 <= min_z && z2 <= min_z && z3 <= min_z) // no cull or clip
			{
				continue;
			}
			else // Near-plane clipping only
			{				
				if(z1 > z2)
				{
					SWAP(v1,v2);
					SWAP(z1,z2);
				}
				if(z1 > z3)
				{
					SWAP(v1,v3);
					SWAP(z1,z3);
				}
				if(z2 > z3)
				{
					SWAP(v2,v3);
					SWAP(z2,z3);
				}

				CULLANDCLIP_GET_POINT_VALUES;
				// now,  z1 < z2 < z3
				
				if(pPoly->m_pMaterial != NULL && pPoly->m_pMaterial->m_imgTexture.GetData()!=NULL)
					bTexture = true;
				else
					bTexture = false;

				// Only v3 is inside the view frustum; clip v1 and v2. (Method: Recalculate v1 and v2 and modify the original values in-place)
				if(z2 <= min_z)
				{
					//1. Coordinates (v1 = intersection of v1-v3 and min_z)
					t1 = (min_z - z1)/(z3 - z1);
					pPoly->m_pVertList[pPoly->m_usVertIndices[v1]][0] = x1 + (x3-x1)*t1;
					pPoly->m_pVertList[pPoly->m_usVertIndices[v1]][1] = y1 + (y3-y1)*t1;
					pPoly->m_pVertList[pPoly->m_usVertIndices[v1]][2] = min_z;

					t2 = (min_z - z2)/(z3 - z2); //(v2 = intersection of v2-v3 and min_z)
					pPoly->m_pVertList[pPoly->m_usVertIndices[v2]][0] = x2 + (x3-x2)*t2;
					pPoly->m_pVertList[pPoly->m_usVertIndices[v2]][1] = y2 + (y3-y2)*t2;
					pPoly->m_pVertList[pPoly->m_usVertIndices[v2]][2] = min_z;

					//2. Texture Coordinates
					if(bTexture)
					{
						//u
						pPoly->m_fTexCoords[0][v1] = pPoly->m_fTexCoords[0][v1] + 
							(pPoly->m_fTexCoords[0][v3] - pPoly->m_fTexCoords[0][v1])*t1;
						pPoly->m_fTexCoords[0][v2] = pPoly->m_fTexCoords[0][v2] + 
							(pPoly->m_fTexCoords[0][v3] - pPoly->m_fTexCoords[0][v2])*t2;
						//v
						pPoly->m_fTexCoords[1][v1] = pPoly->m_fTexCoords[1][v1] + 
							(pPoly->m_fTexCoords[1][v3] - pPoly->m_fTexCoords[1][v1])*t1;
						pPoly->m_fTexCoords[1][v2] = pPoly->m_fTexCoords[1][v2] + 
							(pPoly->m_fTexCoords[1][v3] - pPoly->m_fTexCoords[1][v2])*t2;
					}

					//3. Normal Vectors
					// @@@@@@@@@@ Since lighting has already been calculated, there is no need to process the normals; instead, modify the vertex colors directly.
					// @@@@@@@@@@ If performing clipping before lighting, do not modify the colors. Instead, process the normals (ensure the normals are stored within the polygon class).

					//4. Light Colors
						//4.1 polygon color
					pPoly->m_rgbaColor[v1].GetRGBValuec(r1,g1,b1);
					pPoly->m_rgbaColor[v2].GetRGBValuec(r2,g2,b2);
					pPoly->m_rgbaColor[v3].GetRGBValuec(r3,g3,b3);
					r1 = r1 + (r3-r1)*t1;
					g1 = g1 + (g3-g1)*t1;
					b1 = b1 + (b3-b1)*t1;
					r2 = r2 + (r3-r2)*t2;
					g2 = g2 + (g3-g2)*t2;
					b2 = b2 + (b3-b2)*t2;
					pPoly->m_rgbaColor[v1] = CRGBA(r1,g1,b1,pPoly->m_rgbaColor[v1].a());
					pPoly->m_rgbaColor[v2] = CRGBA(r2,g2,b2,pPoly->m_rgbaColor[v2].a());

						//4.2 specular color
					pPoly->m_rgbaSpecular[v1].GetRGBValuec(r1,g1,b1);
					pPoly->m_rgbaSpecular[v2].GetRGBValuec(r2,g2,b2);
					pPoly->m_rgbaSpecular[v3].GetRGBValuec(r3,g3,b3);
					r1 = r1 + (r3-r1)*t1;
					g1 = g1 + (g3-g1)*t1;
					b1 = b1 + (b3-b1)*t1;
					r2 = r2 + (r3-r2)*t2;
					g2 = g2 + (g3-g2)*t2;
					b2 = b2 + (b3-b2)*t2;
					pPoly->m_rgbaSpecular[v1] = CRGBA(r1,g1,b1,pPoly->m_rgbaSpecular[v1].a());
					pPoly->m_rgbaSpecular[v2] = CRGBA(r2,g2,b2,pPoly->m_rgbaSpecular[v2].a());

				}
				else // Both v2 and v3 are inside the view frustum. The result becomes a quadrilateral, which must be split into two triangles.
				{
					//CPolygon* polygon_new = new CPolygon(*pPoly);
					CVertex4 v1v3_intersection = pPoly->m_pVertList[pPoly->m_usVertIndices[v1]];
					CVector4 v1v2_intersection = pPoly->m_pVertList[pPoly->m_usVertIndices[v1]];
					
					//1. Coordinates
					t1 = (min_z - z1)/(z3 - z1);
					v1v3_intersection[0] = x1 + (x3-x1)*t1;
					v1v3_intersection[1] = y1 + (y3-y1)*t1;
					v1v3_intersection[2] = min_z;
					
					t2 = (min_z - z1)/(z2 - z1);
					v1v2_intersection[0] = x1 + (x2-x1)*t2;
					v1v2_intersection[1] = y1 + (y2-y1)*t2;
					v1v2_intersection[2] = min_z;
					
					//2. Texture Coordinates
					if(bTexture)
					{
						//u
						pPoly->m_fTexCoords[0][v1] = pPoly->m_fTexCoords[0][v1] + 
							(pPoly->m_fTexCoords[0][v3] - pPoly->m_fTexCoords[0][v1])*t1;
						//v
						pPoly->m_fTexCoords[1][v1] = pPoly->m_fTexCoords[1][v1] + 
							(pPoly->m_fTexCoords[1][v3] - pPoly->m_fTexCoords[1][v1])*t1;
					}

					//3. Normal Vectors, do nothing


					//4. Light Colors
						//4.1 polygon color
					pPoly->m_rgbaColor[v1].GetRGBValuec(r1,g1,b1);
					pPoly->m_rgbaColor[v3].GetRGBValuec(r3,g3,b3);
					r1 = r1 + (r3-r1)*t1;
					g1 = g1 + (g3-g1)*t1;
					b1 = b1 + (b3-b1)*t1;
					pPoly->m_rgbaColor[v1] = CRGBA(r1,g1,b1,pPoly->m_rgbaColor[v1].a());

						//4.2 specular color
					pPoly->m_rgbaSpecular[v1].GetRGBValuec(r1,g1,b1);
					pPoly->m_rgbaSpecular[v3].GetRGBValuec(r3,g3,b3);
					r1 = r1 + (r3-r1)*t1;
					g1 = g1 + (g3-g1)*t1;
					b1 = b1 + (b3-b1)*t1;
					pPoly->m_rgbaSpecular[v1] = CRGBA(r1,g1,b1,pPoly->m_rgbaSpecular[v1].a());


					//5. Assignment
					pPoly->m_pVertList[pPoly->m_usVertIndices[v1]] = v1v3_intersection;
					

					//nCulledPolygons++;
				}
			}
		}

		return nCulledPolygons;
	}
}
