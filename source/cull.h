// cull.h: interface for the CCull class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CULL_H_B44356E6_3A58_48E9_909F_465CFC2A87E0)
#define CULL_H_B44356E6_3A58_48E9_909F_465CFC2A87E0


#include "object.h"
#include "camera.h"

namespace Nomad3D
{
	enum ECullType
	{
		NM3D_OUT_OF_FRUSTUM,
		NM3D_FULL_IN_FRUSTUM,
		NM3D_HALF_IN_FRUSTUM
	};

	class CCull
	{
	public:
		virtual ~CCull(){};
		virtual ECullType TestObject(CObject* obj,CCamera* cam) = 0;
	};

	class CCullOutSphere: public CCull
	{
	public:
		ECullType TestObject(CObject* obj,CCamera* cam);
	};
}


#endif // !defined(CULL_H_B44356E6_3A58_48E9_909F_465CFC2A87E0)
