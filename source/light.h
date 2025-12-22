// light.h: interface for the CLight class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(LIGHT_H_B383B95E_604D_4C92_9CB6_3C1D2E38A264)
#define LIGHT_H_B383B95E_604D_4C92_9CB6_3C1D2E38A264

#include "rgb.h"
#include "vector3.h"
#include "object.h"
#include "renderlist.h"
#include "polygon.h"
#include "camera.h"
#include "matrix4.h"
#include "render.h"

namespace Nomad3D
{
	class CObject;

	class CLight
	{
	public:
 		enum ELightState
 		{
 			NM3D_LIGHT_STATE_ON,
 			NM3D_LIGHT_STATE_OFF
 		};
		
		enum ELightType
		{
			NM3D_LIGHT_TYPE_DIRECTION,	//ƽ�й�
			NM3D_LIGHT_TYPE_POINT,		//���Դ
			NM3D_LIGHT_TYPE_SPOT		//�۹��
		};
	private:

#define NM3D_LIGH_UNLIGHTED_VERTEX	0
#define NM3D_LIGH_LIGHTED_VERTEX	1

		//��Դ
		class _Light
		{
		public:
			ELightState state;	// state of light
			int id;				// id of light
			ELightType attr;	// type of light, and extra qualifiers
			
			CRGBA ambient;		// ambient light intensity
			CRGBA diffuse;		// diffuse light intensity
			CRGBA specular;		// specular light intensity
			
			CPoint4  pos,tpos;	// position of light world/transformed
			CVector3 dir,tdir;	// direction of light world/transformed

			float kc, kl, kq;	// attenuation factors
			float spot_angle;	// angle for spot light
			float pf;           // power factor/falloff for spot lights
		};

	public:
		CLight();
		~CLight();
		//�����ɹ����ظձ������Ĺ�Դ�Ĺ�Դ�ı��(Ҳ���ܵĹ�Դ��),ʧ���򷵻�-1
		int CreateLight(
			ELightState	_state,      // state of light
			ELightType	_attr,       // type of light, and extra qualifiers
			CRGBA		_c_ambient,  // ambient light intensity
			CRGBA		_c_diffuse,  // diffuse light intensity
			CRGBA		_c_specular, // specular light intensity
			const CPoint4*	_pos,    // position of light
			const CVector3*	_dir,    // direction of light
			float		_kc,         // attenuation factors
			float		_kl, 
			float		_kq, 
			float		_spot_angle, // angle for spot light
			float		_pf			 // power factor/falloff for spot lights
			);
		const CRGBA& GetGlobalAmbient() const;
		void SetGlobalAmbient(CRGBA& rgba);
		ELightState GetLightState(int n) const;
		void SetLightState(int n, ELightState state);
		void Transform(CMatrix4& mt, int n=-1);
		int LightObject16(CObject* pObj, const CCamera* pCam);
		int LightRenderList16(CRenderList* pRenderList, const CCamera* pCam);
	protected:
		void LightPolygon(CPolygon* pPoly, const CCamera* pCam);
	private:
		CRGBA		m_rgbGlobalAmbient;
		int			m_nNumLights;
		_Light*		m_LightList[NM3D_MAX_LIGHT_COUNT];
		int*		m_pnTempLightVertexList; // flag vertex lighted or unlighted
		CRGBA*		m_prgbaTempColor; //polygon color / vertex color and separate specular color
	};

	extern CLight light;
}


#endif // !defined(LIGHT_H_B383B95E_604D_4C92_9CB6_3C1D2E38A264)
