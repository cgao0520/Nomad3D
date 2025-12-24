#include "nomad3d.h"
#include "canvas.h"
#include <stdlib.h>

namespace Nomad3D
{
    CCanvas::CCanvas()
    {
        m_pCanvas = NULL;
    }

    CCanvas::~CCanvas()
    {
        if (m_pCanvas) // in case not freed in subclass
        {
            free(m_pCanvas);
        }
        
        m_pCanvas = NULL;
    }

    NM3D_BUFFER CCanvas::GetCanvas()
    {
        return m_pCanvas;
    }

    int CCanvas::GetCanvasPitch()
    {
        assert(m_pCanvas);
        return NM3D_WINDOW_WIDTH * sizeof(m_pCanvas[0]);
    }

    void CCanvas::BitBlt()
    {
        // implemnent in subclass
    }

    void CCanvas::ClearCanvas()
    {
        assert(m_pCanvas);
        memset(m_pCanvas, 0, GetCanvasPitch() * NM3D_WINDOW_HEIGHT );
    }
}
