// mesh.h: interface for the CMesh class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MESH_H_F2468E5D_8F32_44E8_ACE4_5A9F725B6D88
#define MESH_H_F2468E5D_8F32_44E8_ACE4_5A9F725B6D88

#include "light.h"
#include <cstring>

namespace Nomad3D
{
#pragma pack(push, packing)
#pragma pack(1)

	class CLight;

	class CMesh  
	{ 
		friend class CLight;
	public:
		CMesh()
		{
			m_pusIndices = NULL;
		}

		CMesh(unsigned short usNumPolys, unsigned char* pusIndices, char cMaterial)
		{
			Assign(usNumPolys, pusIndices, cMaterial);
		}

		~CMesh()
		{
			if(m_pusIndices)
			{
				delete [] m_pusIndices;
				m_pusIndices = NULL;
			}
		}

		void Assign(unsigned short usNumPolys, unsigned char* pusIndices, char cMaterial)
		{
			m_usNumPolys = usNumPolys;
			m_cMaterial = cMaterial;
			m_pusIndices = new unsigned short[usNumPolys];
			memcpy(m_pusIndices, pusIndices, sizeof(unsigned short)*usNumPolys);
		}

		inline void SetMaterial(char cIndex)
		{
			m_cMaterial  = cIndex;
		}
	public: 
		unsigned short  m_usNumPolys;	// Number of ploygons in mesh
		unsigned short* m_pusIndices;	// Ploygon indice
		char			m_cMaterial;	// Material reference, -1 indicates no material
	};
#pragma pack(pop, packing)
}


#endif //MESH_H_F2468E5D_8F32_44E8_ACE4_5A9F725B6D88
