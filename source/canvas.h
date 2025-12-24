#ifndef __CANVAS_H__
#define __CANVAS_H__

#include "config.h"
#include "platform.h"
#include <SDL2/SDL.h>


namespace Nomad3D
{
    class CCanvas
    {
    public:
        CCanvas();
        virtual ~CCanvas();
        NM3D_BUFFER GetCanvas();
        int GetCanvasPitch();
        void ClearCanvas();
        virtual void BitBlt();

    protected:
        NM3D_BUFFER m_pCanvas;
    };
}

#endif // __CANVAS_H__
