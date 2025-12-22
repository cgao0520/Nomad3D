// ms3dobject.h: interface for the CMS3DObject class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MS3DOBJECT_H_B48CED90_DC0E_4CC9_8CCE_A39137653320
#define MS3DOBJECT_H_B48CED90_DC0E_4CC9_8CCE_A39137653320

#include "matrix4.h"
#include "object.h"
#include "mesh.h"
#include "material.h"
#include "quaternion.h"
#include "timer.h"

namespace Nomad3D
{
	class CMS3DObject : public CObject
	{
	public:
		CMS3DObject();
		virtual ~CMS3DObject();
	public:
		bool ReadObjectFile(const char* szFilename);
		bool SetupSkeleton();
		void CalcLocalMatrix(const float* fRotation, const float* fPosition, CMatrix4& mat);
		void SetPolygonMaterialAndVertList();
	};
}

#endif //MS3DOBJECT_H_B48CED90_DC0E_4CC9_8CCE_A39137653320
