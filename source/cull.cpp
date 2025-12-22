// cull.cpp: implementation of the CCull class.
//
//////////////////////////////////////////////////////////////////////

#include "cull.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace Nomad3D
{

	ECullType CCullOutSphere::TestObject(CObject* obj,CCamera* cam)
	{
		return NM3D_FULL_IN_FRUSTUM;

		assert(obj && cam);
		CPoint3 pos = obj->GetPos();
		float	r = obj->GetMaxRadius();

		CVector4 left(pos.x()-r,pos.y(),pos.z()),
			right(pos.x()+r,pos.y(),pos.z()),
			top(pos.x(),pos.y()+r,pos.z()),
			bottom(pos.x(),pos.y()-r,pos.z()),
			face(pos.x(),pos.y(),pos.z()+r),
			back(pos.x(),pos.y(),pos.z()-r);

		CMatrix4 m = cam->GetCameraMatrix();

		left *= m;
		right *= m;
		top *= m;
		bottom *= m;
		face *= m;
		back *= m;

		//////////////////////////////////////////////////////////////////////////
		//Divide w
		for(int i=0; i<3; i++)
		{
			left[i] /= left[3];
			right[i] /= right[3];
			top[i] /= top[3];
			bottom[i] /= bottom[3];
			face[i] /= face[3];
			back[i] /= back[3];
		}
/*
		float l = left[0];
		float ri = right[0];
		float t = top[1];
		float b = bottom[1];
		float f = face[2];
		float ba = back[2];
*/		
		if(	left[0] >1 || right[0] < -1 || 
			top[1]	< -1 || bottom[1] >1 ||
			face[2] > 1 || back[2] <-1
			//face[2] < -1 || back[2] > 1
			)
			return NM3D_OUT_OF_FRUSTUM;
		else
			return NM3D_FULL_IN_FRUSTUM;
	}
}

