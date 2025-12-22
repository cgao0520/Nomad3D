#ifndef NOMAD3D_H_9676F8D5_52E6_4a3a_B090_BB5F8C0149AA
#define NOMAD3D_H_9676F8D5_52E6_4a3a_B090_BB5F8C0149AA

#include "config.h"
#include "platform.h"
#include "log.h"
#include "constant.h"
#include "vector3.h"
#include "matrix4.h"
#include "image.h"
#include "rgb.h"
#include "render.h"
#include "camera.h"
#include "quaternion.h"
#include "ms3dobject.h"
#include "scene.h"
#include "picture.h"
#include "player.h"
#include "font.h"
#include "renderlist.h"

#if NM3D_DEBUG_ON
#include <assert.h>
#endif

namespace Nomad3D
{
	//extern CRender render;
}

#if NM3D_DEBUG_ON
	extern CLog g_log;
#endif


#endif //NOMAD3D_H_9676F8D5_52E6_4a3a_B090_BB5F8C0149AA
