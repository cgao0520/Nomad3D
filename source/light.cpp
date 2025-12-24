// light.cpp: implementation of the CLight class.
//
//////////////////////////////////////////////////////////////////////

#include "light.h"
#include <float.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace Nomad3D
{

#define NM3D_LIGH_UNLIGHTED_VERTEX	0
#define NM3D_LIGH_LIGHTED_VERTEX	1

	CLight light;

	CLight::CLight()
	{
		m_nNumLights = 0;
		for(int i=0; i<NM3D_MAX_LIGHT_COUNT; i++)
			m_LightList[i] = NULL;

		m_rgbGlobalAmbient = CRGBA(0, 0, 0, 1);

		m_pnTempLightVertexList = NULL;
		m_prgbaTempColor = NULL;
	}

	CLight::~CLight()
	{
		for(int i=0; i<NM3D_MAX_LIGHT_COUNT; i++)
		{
			if(m_LightList[i] != NULL)
			{
				delete m_LightList[i];
			}
		}

		if(m_pnTempLightVertexList)
			delete[] m_pnTempLightVertexList;
		m_pnTempLightVertexList = NULL;

		if(m_prgbaTempColor)
			delete[] m_prgbaTempColor;
		m_prgbaTempColor = NULL;
	}

	int CLight::CreateLight(
			ELightState	_state,			// state of light
			ELightType	_attr,			// type of light, and extra qualifiers
			CRGBA		_c_ambient,		// ambient light intensity
			CRGBA		_c_diffuse,		// diffuse light intensity
			CRGBA		_c_specular,	// specular light intensity
			const CPoint4*	_pos,		// position of light
			const CVector3*	_dir,		// direction of light
			float		_kc,			// attenuation factors
			float		_kl,
			float		_kq,
			float		_spot_angle,	// angle for spot light
			float		_pf				// power factor/falloff for spot lights
			)
	{
		if(m_nNumLights >= NM3D_MAX_LIGHT_COUNT)
			return -1;
		
		int index = m_nNumLights;
		m_LightList[index] = new _Light;
		if(!m_LightList[index])
			return -1;

		m_LightList[index]->state	= _state;      // state of light
		m_LightList[index]->id		= index;       // id of light
		m_LightList[index]->attr	= _attr;       // type of light, and extra qualifiers
		
		m_LightList[index]->ambient	= _c_ambient;  // ambient light intensity
		m_LightList[index]->diffuse	= _c_diffuse;  // diffuse light intensity
		m_LightList[index]->specular= _c_specular; // specular light intensity
		
		m_LightList[index]->kc		= _kc;         // constant, linear, and quadratic attenuation factors
		m_LightList[index]->kl		= _kl;   
		m_LightList[index]->kq		= _kq;   
		
		if (_pos)
		{
			m_LightList[index]->pos = *_pos;  // position of light
			m_LightList[index]->tpos= *_pos;
		} 
		
		if (_dir)
		{
			m_LightList[index]->dir = *_dir;  // direction of light
			
			// normalize it
			m_LightList[index]->dir.Normalize();
			m_LightList[index]->tdir = m_LightList[index]->dir; 
		} 
		
		m_LightList[index]->spot_angle  = Cos(_spot_angle); // angle for spot light
		m_LightList[index]->pf          = _pf; // power factor/falloff for spot lights
		
		m_nNumLights++;
		
		return(m_nNumLights);
	}
	
	const CRGBA& CLight::GetGlobalAmbient() const
	{
		return m_rgbGlobalAmbient;
	}

	void CLight::SetGlobalAmbient(CRGBA& rgba)
	{
		m_rgbGlobalAmbient = rgba;
	}

	void CLight::SetLightState(int n, ELightState state)
	{
		if( n >= m_nNumLights || n < 0)
			return;
		else
			m_LightList[n]->state = state;
	}

	CLight::ELightState CLight::GetLightState(int n) const
	{
		if( n >= m_nNumLights || n < 0)
			return NM3D_LIGHT_STATE_OFF;
		else
			return m_LightList[n]->state;
	}

	void CLight::Transform(CMatrix4& mt, int n)
	{
		if( n >= m_nNumLights )
			return;

		_Light* pLight = NULL;

		if(n < 0)
		{
			for(int i=0; i<m_nNumLights; i++)
			{
				pLight = m_LightList[i];
				pLight->tpos = pLight->pos * mt;
				pLight->tdir = pLight->dir * mt;
				pLight->tdir.Normalize();
			}
		}
		else
		{
			pLight = m_LightList[n];
			pLight->tpos = pLight->pos * mt;
			pLight->tdir = pLight->dir * mt;
			pLight->tdir.Normalize();
		}

	}

	//////////////////////////////////////////////////////////////////////////
	int CLight::LightObject16(CObject* pObj, const CCamera* pCam)
	{
		if(!pObj || !pCam)
			return 0;

// 		if (pObj->m_nState is culled)
// 			return 0;

		unsigned int i = 0;
		int j = 0;
		int nNumPolys;
		CPolygon* pPoly = NULL;

		int nNumVerts = pObj->GetNumVerts();
		m_pnTempLightVertexList = new int[nNumVerts];
		memset(m_pnTempLightVertexList, NM3D_LIGH_UNLIGHTED_VERTEX, nNumVerts*sizeof(int));
		m_prgbaTempColor = new CRGBA[nNumVerts<<1];

		for(i=0; i<pObj->m_unNumMeshes; i++)
		{
			CMesh* pMesh = pObj->m_pMeshList + i;
			nNumPolys = pMesh->m_usNumPolys;

			for(j=0; j<nNumPolys; j++)
			{
				pPoly = pObj->m_pPolyList + pMesh->m_pusIndices[j];
				if(pPoly->m_nState & NM3D_BACK_POLYGON)
					continue;
				LightPolygon(pPoly, pCam);
			}//for polygon
		}//for mesh

		if(m_pnTempLightVertexList)
			delete[] m_pnTempLightVertexList;
		m_pnTempLightVertexList = NULL;

		if(m_prgbaTempColor)
			delete[] m_prgbaTempColor;
		m_prgbaTempColor = NULL;

		return 1;
	}

	int CLight::LightRenderList16(CRenderList* pRenderList, const CCamera* pCam)
	{
		int nNumPolys = pRenderList->GetNumPolys();
		CPolygon* pPolygon = pRenderList->GetPolyList();
		int i;
		CPolygon* pPoly = NULL;

		int nNumVerts = pRenderList->GetNumVerts();
		m_pnTempLightVertexList = new int[nNumVerts];
		memset(m_pnTempLightVertexList, NM3D_LIGH_UNLIGHTED_VERTEX, nNumVerts*sizeof(int));
		m_prgbaTempColor = new CRGBA[nNumVerts<<1];

		for(i=0; i<nNumPolys; i++)
		{
			pPoly = pPolygon + i;
			if(pPoly->m_nState & NM3D_BACK_POLYGON)
				continue;
			LightPolygon(pPoly, pCam);
		}

		if(m_pnTempLightVertexList)
			delete[] m_pnTempLightVertexList;
		m_pnTempLightVertexList = NULL;

		if(m_prgbaTempColor)
			delete[] m_prgbaTempColor;
		m_prgbaTempColor = NULL;

		return nNumPolys;
	}

	void CLight::LightPolygon(CPolygon* pPoly, const CCamera* pCam)
	{
		if(!pPoly || !pCam || pPoly->GetState() & NM3D_BACK_POLYGON || pPoly->GetState() & NM3D_CULLED_POLYGON)
			return;

#define LIGHT_POLYGON_FACE_MODE \
		(render.GetRenderType() != CRender::NM3D_RENDER_TYPE_GOURAUD && \
		render.GetRenderType() != CRender::NM3D_RENDER_TYPE_GOURAUD_TEXTURE && \
		render.GetRenderType() != CRender::NM3D_RENDER_TYPE_GOURAUD_TEXTURE_SS && \
		render.GetRenderType() != CRender::NM3D_RENDER_TYPE_GOURAUD_LaMoth && \
		render.GetRenderType() != CRender::NM3D_RENDER_TYPE_GOURAUD_TEXTURE_LaMoth)

		bool bFaceMode = LIGHT_POLYGON_FACE_MODE;
		
		int r,g,b,clr_r,clr_g,clr_b;
		CMaterial* pMaterial = pPoly->m_pMaterial;
		CVertex4* pVertList = pPoly->m_pVertList;
		unsigned short* pusVertIndices = pPoly->m_usVertIndices;

		//Specular part Color
		int specular_r,specular_g,specular_b;

		//Step(1) Add Emission from Material
		r = pMaterial->m_fEmissive[0] * 255;
		g = pMaterial->m_fEmissive[1] * 255;
		b = pMaterial->m_fEmissive[2] * 255;

		//Step(2) Add Global Ambient
		r += m_rgbGlobalAmbient.rc() * pMaterial->m_fAmbient[0];
		g += m_rgbGlobalAmbient.gc() * pMaterial->m_fAmbient[1];
		b += m_rgbGlobalAmbient.bc() * pMaterial->m_fAmbient[2];

		//Step(3) Add every light's contribution
		int i=0;
		CVector3 n[3]; // n[0] for flat shading. and n[0]~n[2] for gouraud shading
		for(; i<3; i++) // 0 ~ 2 vertex of polygon
		{
			if(!bFaceMode)
			{// one color per vertex
				if(m_pnTempLightVertexList[pPoly->m_usVertIndices[i]] == NM3D_LIGH_LIGHTED_VERTEX)
				{
					pPoly->m_rgbaColor[i] = m_prgbaTempColor[(pPoly->m_usVertIndices[i]<<1)];
					pPoly->m_rgbaSpecular[i] = m_prgbaTempColor[(pPoly->m_usVertIndices[i]<<1) + 1];
					continue;
				}
				else
				{
					m_pnTempLightVertexList[pPoly->m_usVertIndices[i]] = NM3D_LIGH_LIGHTED_VERTEX;
				}
			}

			clr_r = clr_g = clr_b = 0;
			specular_r = specular_g = specular_b = 0;
			int k;
			_Light* pLight = NULL;
			CVector4 pv; //vertex(for gouraud/flat shading) or average value of polygon's vertex(only for flat shading)
			
			if(bFaceMode)
			{
				CVector4 v11,v22,v33;
				v11 = pVertList[pusVertIndices[0]];
				v22 = pVertList[pusVertIndices[1]];
				v33 = pVertList[pusVertIndices[2]];
				CVector3 v1((v11-v22).Get());
				CVector3 v2((v11-v33).Get());
				n[0] = Cross(v1,v2);
				n[0].Normalize();
#if NM3D_LIGHT_FLAT_SHADING_AVERAGE_POINT
				pv = (v11+v22+v33)/3;
#else
				pv = v11;
#endif
			}
			else
			{
				n[i] = pVertList[pusVertIndices[i]].GetNormal(); 
				//no need to normalize normal vector, it has always be normalized
				n[i].Normalize();
				pv = pVertList[pusVertIndices[i]];
			}
			
			for(k=0; k<m_nNumLights; k++)
			{
				pLight = m_LightList[k];
				
				if(pLight->state == NM3D_LIGHT_STATE_OFF)
					continue;
				
				switch(pLight->attr)
				{
				case NM3D_LIGHT_TYPE_DIRECTION:
					{
						//(1) Add Ambient part
						clr_r += pLight->ambient.rc() * pMaterial->m_fAmbient[0];
						clr_g += pLight->ambient.gc() * pMaterial->m_fAmbient[1];
						clr_b += pLight->ambient.bc() * pMaterial->m_fAmbient[2];
						
						//(2) Add Diffuse part
						CVector4 L = pLight->tdir;//pVertList[pusVertIndices[0]] - pLight->tdir;
						//L.Normalize(); //L = pLight->tdir is already normalized
						float dotLn =  Dot4(L,n[i]);
						if( dotLn > 0)//dotLn>0 only when this plane is face to the light(catch light)
						{
							clr_r += (dotLn * pLight->diffuse.rc() * pMaterial->m_fDiffuse[0]);
							clr_g += (dotLn * pLight->diffuse.gc() * pMaterial->m_fDiffuse[1]);
							clr_b += (dotLn * pLight->diffuse.bc() * pMaterial->m_fDiffuse[2]);
#if (NM3D_LIGHT_SPECULAR_ON)
							//only when dotLn > 0, that will calculate (3)
							//(3) Add Specular part
							CVector4 LL = pLight->tdir - pv;//pVertList[pusVertIndices[i]];
							CVector4 VV = pCam->GetPosition() - pv;//pVertList[pusVertIndices[i]];
							LL.Normalize();VV.Normalize();
							CVector4 s = LL + VV;
							float ss = MAX(Dot4(s,n[i]),0);
							float ms = ss;
							for(int t=1; t<(int)pMaterial->m_fShininess; t++)
								ms *= ss;
							specular_r += ms * pLight->specular.rc() * pMaterial->m_fSpecular[0];
							specular_g += ms * pLight->specular.gc() * pMaterial->m_fSpecular[1];
							specular_b += ms * pLight->specular.bc() * pMaterial->m_fSpecular[2];
#endif //NM3D_LIGHT_SPECULAR_ON
						}
					}
					break;
				case NM3D_LIGHT_TYPE_POINT:
					{
						// distance between vertex to the light, for calculating the attenuation 
						float d = (/*pVertList[pusVertIndices[i]]*/pv - pLight->tpos).Length();
						
						//(0) Calculate Attenuation
						float atten = 1.0f/(pLight->kc + pLight->kl*d + pLight->kq*d*d);
						
						//(1) Add Ambient part
						clr_r += atten * pLight->ambient.rc() * pMaterial->m_fAmbient[0];
						clr_g += atten * pLight->ambient.gc() * pMaterial->m_fAmbient[1];
						clr_b += atten * pLight->ambient.bc() * pMaterial->m_fAmbient[2];
						
						//(2) Add Diffuse part
						CVector4 L = pLight->tpos;///*tdir*/;//pVertList[pusVertIndices[0]] - pLight->tpos;//@@@
						L.Normalize();
						float dotLn =  Dot4(L,n[i]);
						if( dotLn > 0) // Light on the surface only when dotLn > 0, otherwise, light is on the back side of surface
						{
							//(2) Add Diffuse part
							clr_r += atten * dotLn * pLight->diffuse.rc() * pMaterial->m_fDiffuse[0];
							clr_g += atten * dotLn * pLight->diffuse.gc() * pMaterial->m_fDiffuse[1];
							clr_b += atten * dotLn * pLight->diffuse.bc() * pMaterial->m_fDiffuse[2];
#if (NM3D_LIGHT_SPECULAR_ON)
							//only when dotLn > 0, that will calculate (3)
							//(3) Add Specular part
							CVector4 LL = pLight->tpos - pv;//pVertList[pusVertIndices[i]];
							CVector4 VV = pCam->GetPosition() - pv;//pVertList[pusVertIndices[i]];
							LL.Normalize();VV.Normalize();
							CVector4 s = LL + VV;
							float ss = MAX(Dot4(s,n[i]),0);
							float ms = ss;
							for(int t=1; t<(int)pMaterial->m_fShininess; t++)
								ms *= ss;
							specular_r += atten * ms * pLight->specular.rc() * pMaterial->m_fSpecular[0];
							specular_g += atten * ms * pLight->specular.gc() * pMaterial->m_fSpecular[1];
							specular_b += atten * ms * pLight->specular.bc() * pMaterial->m_fSpecular[2];
#endif //NM3D_LIGHT_SPECULAR_ON
						}
					}
					break;
				case NM3D_LIGHT_TYPE_SPOT:
					{
						//(0) Calculate Attenuation
						CVector4 pl = pv - pLight->tpos;
						float d = pl.Length(); // distance between vertex to the light, for calculating the attenuation
						float atten = 1.0f/(pLight->kc + pLight->kl*d + pLight->kq*d*d);
						
						//(0.5) Calculate SpotLight Effection
						pl = -pl;
						pl.Normalize();
						float dpr = Dot4(pl,pLight->tdir);
						if(dpr < pLight->spot_angle) // out of the angle of spot light
							break;
						dpr = MAX(dpr,0);
						float spot_effect = dpr;
						int spot_exponent = (pLight->pf+0.5);
						for(int _j=1; _j<spot_exponent; _j++)
							spot_effect *= dpr;
					 
						//(1) Add Ambient part
						clr_r += spot_effect * atten * pLight->ambient.rc() * pMaterial->m_fAmbient[0];
						clr_g += spot_effect * atten * pLight->ambient.gc() * pMaterial->m_fAmbient[1];
						clr_b += spot_effect * atten * pLight->ambient.bc() * pMaterial->m_fAmbient[2];
						
						//(2) Add Diffuse part
						CVector4 L = pLight->tpos;///*tdir*/;//pVertList[pusVertIndices[0]] - pLight->tpos;//@@@
						L.Normalize();
						float dotLn =  Dot4(L,n[i]);
						if( dotLn > 0) // Light on the surface only when dotLn > 0, otherwise, light is on the back side of surface
						{
							//(2) Add Diffuse part
							clr_r += spot_effect * atten * dotLn * pLight->diffuse.rc() * pMaterial->m_fDiffuse[0];
							clr_g += spot_effect * atten * dotLn * pLight->diffuse.gc() * pMaterial->m_fDiffuse[1];
							clr_b += spot_effect * atten * dotLn * pLight->diffuse.bc() * pMaterial->m_fDiffuse[2];
#if (NM3D_LIGHT_SPECULAR_ON)
							//only when dotLn > 0, that will calculate (3)
							//(3) Add Specular part
							CVector4 LL = pLight->tpos - pv;//pVertList[pusVertIndices[i]];
							CVector4 VV = pCam->GetPosition() - pv;//pVertList[pusVertIndices[i]];
							LL.Normalize();VV.Normalize();
							CVector4 s = LL + VV;
							float ss = MAX(Dot4(s,n[i]),0);
							float ms = ss;
							for(int t=1; t<(int)pMaterial->m_fShininess; t++)
								ms *= ss;
							specular_r += spot_effect * atten * ms * pLight->specular.rc() * pMaterial->m_fSpecular[0];
							specular_g += spot_effect * atten * ms * pLight->specular.gc() * pMaterial->m_fSpecular[1];
							specular_b += spot_effect * atten * ms * pLight->specular.bc() * pMaterial->m_fSpecular[2];
#endif //NM3D_LIGHT_SPECULAR_ON
						}
					
					}
					break;
				default:
					break;
				}//switch
			}//for light
#if (NM3D_LIGHT_SPECULAR_ON) && (NM3D_LIGHT_SEPARATE_SPECULAR)
			CRGBA res_specular_color(MIN(specular_r,255),MIN(specular_g,255),MIN(specular_b,255),pPoly->m_rgbaSpecular[i].a());
			pPoly->m_rgbaSpecular[i] = res_specular_color;
			m_prgbaTempColor[(pPoly->m_usVertIndices[i]<<1)+1] = res_specular_color;
#elif (NM3D_LIGHT_SPECULAR_ON) && !(NM3D_LIGHT_SEPARATE_SPECULAR)
			clr_r += specular_r;
			clr_g += specular_g;
			clr_b += specular_b;
#endif
			CRGBA res_color(MIN(clr_r+r,255),MIN(clr_g+g,255),MIN(clr_b+b,255),pPoly->m_rgbaColor[i].a());
			pPoly->m_rgbaColor[i] = res_color;
			m_prgbaTempColor[(pPoly->m_usVertIndices[i]<<1)] = res_color;

			//only it is gouraud shading, just need to calculate pPoly->m_rColor[0] to pPoly->m_rColor[2] as vertex color
			if(bFaceMode)
				break; 
		}//for i vertex
	}//LightPolygon()
}//namespace Nomad3D
