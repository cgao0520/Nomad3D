#ifndef _SDL_CANVAS_H_
#define _SDL_CANVAS_H_

#include <SDL2/SDL.h>
#include "nomad3d.h"

using namespace Nomad3D;

class CSDLCanvas : public CCanvas
{
public:
    CSDLCanvas();
    ~CSDLCanvas();
    void BitBlt();
    void Init();
protected:
    SDL_Window *m_pSDLWindow;
    SDL_Renderer *m_pSDLRenderer;
    SDL_Texture *m_pSDLTexture;
};

#endif // _SDL_CANVAS_H_
