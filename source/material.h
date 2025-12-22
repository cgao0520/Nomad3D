// material.h: interface for the CMaterial class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MATERIAL_H_F9A557A2_3242_4501_8EC2_BDE3107C7A9F
#define MATERIAL_H_F9A557A2_3242_4501_8EC2_BDE3107C7A9F


#include "image.h"
#include <memory.h>

namespace Nomad3D
{
#pragma pack(push, packing)
#pragma pack(1)

	class CLight;

	class CMaterial  
	{
		friend class CLight;
	public:
		CMaterial()
		{

		}
		//virtual ~CMaterial();
		void Assign(const float* pfAmbient, const float* pfDiffuse, const float* pfSpecular, const float* pfEmissive,
					float fShininess, float fTransparency, const char* szTextureFileName)
		{
			memcpy(m_fAmbient,pfAmbient,sizeof(m_fAmbient));
			memcpy(m_fDiffuse,pfDiffuse,sizeof(m_fDiffuse));
			memcpy(m_fSpecular,pfSpecular,sizeof(m_fSpecular));
			memcpy(m_fEmissive,pfEmissive,sizeof(m_fEmissive));
			m_fShininess = fShininess;
			m_fTransparency = fTransparency;
			
			if(szTextureFileName)
			{
				m_imgTexture.LoadTexture(szTextureFileName);
			}
		}
	public:
		float m_fAmbient[4];
		float m_fDiffuse[4];
		float m_fSpecular[4];
		float m_fEmissive[4];
		float m_fShininess;		//1.0f ~ 128.0f
		float m_fTransparency;	//0.0f ~ 1.0f
		CImage m_imgTexture;	//Texture image
	};

#pragma pack(pop, packing)
}

#endif //MATERIAL_H_F9A557A2_3242_4501_8EC2_BDE3107C7A9F
